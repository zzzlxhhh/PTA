#ifdef DEV
//优化中的代码
#else
//原始代码
#endif

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

# include "QuEST.h"
# include "QuEST_precision.h"
# include "QuEST_internal.h"    // purely to resolve getQuESTDefaultSeedKey
# include "mt19937ar.h"

# include <stdlib.h>
# include <stdio.h>
# include <math.h>

# include "QuEST.h"

# define REDUCE_SHARED_SIZE 512
# define DEBUG 0


#define THREADS_PER_CUDA_BLOCK 256
#define TASKS_PER_KERNEL_FUC 32
//__constant__ qreal recRoot2 = 1.0 / sqrt(2.0);//dynamic initialization is computed at runtime
__constant__ qreal recRoot2 = 0.7071067811865475;



#ifdef __cplusplus
extern "C" {
#endif

//hadamardKernel单gpu的多个测试版本
#ifdef DEV
	__global__ void statevec_hadamardKernel_v0(Qureg qureg, const int targetQubit) {
		// ----- sizes
		long long int sizeBlock,                                           // size of blocks
			sizeHalfBlock;                                       // size of blocks halved
	   // ----- indices
		long long int thisBlock,                                           // current block
			indexUp, indexLo;                                     // current index and corresponding index in lower half block

	   // ----- temp variables
		qreal   stateRealUp, stateRealLo,                             // storage for previous state values
			stateImagUp, stateImagLo;                             // (used in updates)
	 // ----- temp variables
		long long int thisTask;                                   // task based approach for expose loop with small granularity
		const long long int numTasks = qureg.numAmpsPerChunk >> 1;

		sizeHalfBlock = 1LL << targetQubit;                               // size of blocks halved
		sizeBlock = 2LL * sizeHalfBlock;                           // size of blocks

		qreal *stateVecReal = qureg.deviceStateVec.real;
		qreal *stateVecImag = qureg.deviceStateVec.imag;

		qreal recRoot2 = 1.0 / sqrt(2.0);

		thisTask = blockIdx.x*blockDim.x + threadIdx.x;
		if (thisTask >= numTasks) return;

		thisBlock = thisTask / sizeHalfBlock;
		indexUp = thisBlock * sizeBlock + thisTask % sizeHalfBlock;
		indexLo = indexUp + sizeHalfBlock;

		// store current state vector values in temp variables
		stateRealUp = stateVecReal[indexUp];
		stateImagUp = stateVecImag[indexUp];

		stateRealLo = stateVecReal[indexLo];
		stateImagLo = stateVecImag[indexLo];

		stateVecReal[indexUp] = recRoot2 * (stateRealUp + stateRealLo);
		stateVecImag[indexUp] = recRoot2 * (stateImagUp + stateImagLo);

		stateVecReal[indexLo] = recRoot2 * (stateRealUp - stateRealLo);
		stateVecImag[indexLo] = recRoot2 * (stateImagUp - stateImagLo);
	}

	void statevec_hadamard_v0(Qureg qureg, const int targetQubit)
	{
		int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk >> 1) / THREADS_PER_CUDA_BLOCK);
		statevec_hadamardKernel_v0 << <CUDABlocks, THREADS_PER_CUDA_BLOCK >> > (qureg, targetQubit);
	}

	// 接收预计算的参数
	__global__ void statevec_hadamardKernel_v1(
		Qureg qureg,
		const long long int numTasks, 
		const long long int sizeHalfBlock, 
		const long long int sizeBlock
	) {
		qreal *stateVecReal = qureg.deviceStateVec.real;
		qreal *stateVecImag = qureg.deviceStateVec.imag;

		qreal  stateRealUp, stateRealLo, stateImagUp, stateImagLo;

		long long int thisTask = blockIdx.x*blockDim.x + threadIdx.x;
		if (thisTask >= numTasks) return;

		long long int thisBlock = thisTask / sizeHalfBlock;
		long long int indexUp = thisBlock * sizeBlock + thisTask % sizeHalfBlock;
		long long int indexLo = indexUp + sizeHalfBlock;

		
		stateRealUp = stateVecReal[indexUp];
		stateImagUp = stateVecImag[indexUp];

		stateRealLo = stateVecReal[indexLo];
		stateImagLo = stateVecImag[indexLo];

		stateVecReal[indexUp] = recRoot2 * (stateRealUp + stateRealLo);
		stateVecImag[indexUp] = recRoot2 * (stateImagUp + stateImagLo);

		stateVecReal[indexLo] = recRoot2 * (stateRealUp - stateRealLo);
		stateVecImag[indexLo] = recRoot2 * (stateImagUp - stateImagLo);
	}

	void statevec_hadamard_v1(Qureg qureg, const int targetQubit)
	{
		const long long int numTasks = qureg.numAmpsPerChunk >> 1;
		const long long int sizeHalfBlock = 1LL << targetQubit;
		const long long int sizeBlock = 2LL * sizeHalfBlock;

		int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk >> 1) / THREADS_PER_CUDA_BLOCK);
		statevec_hadamardKernel_v1 << <CUDABlocks, THREADS_PER_CUDA_BLOCK >> > (
			qureg, numTasks,sizeHalfBlock,sizeBlock);
	}
	
	// 一个核函数处理 THREADS_PER_CUDA_BLOCK(2,4,8,...) 个任务
	__global__ void statevec_hadamardKernel_v2(
		Qureg qureg,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock
	) {
		qreal  stateRealUp, stateRealLo, stateImagUp, stateImagLo;

		qreal *stateVecReal = qureg.deviceStateVec.real;
		qreal *stateVecImag = qureg.deviceStateVec.imag;


		long long int thisTask = (blockIdx.x*blockDim.x + threadIdx.x)*TASKS_PER_KERNEL_FUC;
		/*
		https://devblogs.nvidia.com/how-access-global-memory-efficiently-cuda-c-kernels/
		For large strides, the effective bandwidth is poor regardless of architecture version.
		This should not be surprising:
		when concurrent threads simultaneously access memory addresses 
		that are very far apart in physical memory,
		then there is no chance for the hardware to combine the accesses.
		*/
		if (thisTask >= numTasks) return;

		long long int thisBlock = thisTask / sizeHalfBlock;
		long long int indexUp = thisBlock * sizeBlock + thisTask % sizeHalfBlock;
		long long int indexLo = indexUp + sizeHalfBlock;

		for (int i = 0; i < TASKS_PER_KERNEL_FUC; i++) {
			stateRealUp = stateVecReal[indexUp + i];
			stateImagUp = stateVecImag[indexUp + i];

			stateRealLo = stateVecReal[indexLo + i];
			stateImagLo = stateVecImag[indexLo + i];

			stateVecReal[indexUp + i] = recRoot2 * (stateRealUp + stateRealLo);
			stateVecImag[indexUp + i] = recRoot2 * (stateImagUp + stateImagLo);

			stateVecReal[indexLo + i] = recRoot2 * (stateRealUp - stateRealLo);
			stateVecImag[indexLo + i] = recRoot2 * (stateImagUp - stateImagLo);
		}
		
	}
	void statevec_hadamard_v2(Qureg qureg, const int targetQubit)
	{
		const long long int numTasks = qureg.numAmpsPerChunk >> 1;
		const long long int sizeHalfBlock = 1LL << targetQubit;
		const long long int sizeBlock = 2LL * sizeHalfBlock;
		if (sizeHalfBlock < TASKS_PER_KERNEL_FUC) {
			int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk >> 1) / THREADS_PER_CUDA_BLOCK);
			statevec_hadamardKernel_v0 << <CUDABlocks, THREADS_PER_CUDA_BLOCK >> > (qureg, targetQubit);
		}
		else {
			int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk >> 1) / (TASKS_PER_KERNEL_FUC*THREADS_PER_CUDA_BLOCK));
			statevec_hadamardKernel_v2 << <CUDABlocks, THREADS_PER_CUDA_BLOCK >> > (qureg, numTasks, sizeHalfBlock, sizeBlock);
		}
		
	}

	// 使用线程块共享的共享内存
	__global__ void statevec_hadamardKernel_v3(
		Qureg qureg,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock
	) {
		__shared__ qreal slice[4][THREADS_PER_CUDA_BLOCK];

		qreal *stateVecReal = qureg.deviceStateVec.real;
		qreal *stateVecImag = qureg.deviceStateVec.imag;


		long long int thisTask = blockIdx.x*blockDim.x + threadIdx.x;
		if (thisTask >= numTasks) return;

		long long int thisBlock = thisTask / sizeHalfBlock;
		long long int indexUp = thisBlock * sizeBlock + thisTask % sizeHalfBlock;
		long long int indexLo = indexUp + sizeHalfBlock;

		/*
		qreal  stateRealUp, stateRealLo, stateImagUp, stateImagLo;
		stateRealUp = stateVecReal[indexUp];
		stateImagUp = stateVecImag[indexUp];
		stateRealLo = stateVecReal[indexLo];
		stateImagLo = stateVecImag[indexLo];
		*/
		slice[0][threadIdx.x] = stateVecReal[indexUp];
		slice[1][threadIdx.x] = stateVecImag[indexUp];
		slice[2][threadIdx.x] = stateVecReal[indexLo];
		slice[3][threadIdx.x] = stateVecImag[indexLo];

		__syncthreads();

		stateVecReal[indexUp] = recRoot2 * (slice[0][threadIdx.x] + slice[2][threadIdx.x]);
		stateVecImag[indexUp] = recRoot2 * (slice[1][threadIdx.x] + slice[3][threadIdx.x]);

		stateVecReal[indexLo] = recRoot2 * (slice[0][threadIdx.x] - slice[2][threadIdx.x]);
		stateVecImag[indexLo] = recRoot2 * (slice[1][threadIdx.x] - slice[3][threadIdx.x]);
	}

	void statevec_hadamard_v3(Qureg qureg, const int targetQubit)
	{
		const long long int numTasks = qureg.numAmpsPerChunk >> 1;
		const long long int sizeHalfBlock = 1LL << targetQubit;
		const long long int sizeBlock = 2LL * sizeHalfBlock;

		int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk >> 1) / THREADS_PER_CUDA_BLOCK);
		if (sizeHalfBlock < THREADS_PER_CUDA_BLOCK) {
			statevec_hadamardKernel_v0 << <CUDABlocks, THREADS_PER_CUDA_BLOCK >> > (qureg, targetQubit);
		}
		else {
			statevec_hadamardKernel_v3 << <CUDABlocks, THREADS_PER_CUDA_BLOCK,
				4* THREADS_PER_CUDA_BLOCK*sizeof(qreal)>> > (
					qureg, numTasks, sizeHalfBlock, sizeBlock);
		}
	}

#endif

//多gpu支持
#ifdef DEV
	void setDevice(Qureg *qureg, int deviceID) {
		if (deviceID >= qureg->numChunks || deviceID<0) {
			exit(EXIT_FAILURE);
		}
		qureg->chunkId = deviceID;
		qureg->deviceStateVec = qureg->deviceStateVecList[deviceID];
		qureg->devicePairStateVec = qureg->devicePairStateVecList[deviceID];
		qureg->firstLevelReduction = qureg->firstLevelReductionList[deviceID];
		qureg->secondLevelReduction = qureg->secondLevelReductionList[deviceID];
		CUDA_CALL(cudaSetDevice(deviceID));
		//printf("setting qureg on device %d, deviceStateVec.real %x\n", deviceID, qureg->deviceStateVec.real);
	}
	
	void sycAllDevices(Qureg qureg) {
		int originSeviceID = qureg.chunkId;
		for (int i = 0; i < qureg.numChunks; i++)
		{
			CUDA_CALL(cudaSetDevice(i));
			CUDA_CALL(cudaDeviceSynchronize());
		}
		setDevice(&qureg, originSeviceID);
	}

	void getDeviceArrHead(qreal *deviceArr) {
		qreal hostArr[HEAD_SIZE];
		cudaMemcpy(hostArr, deviceArr, HEAD_SIZE * sizeof(qreal), cudaMemcpyDeviceToHost);
		printf("device arr from %x is\n", deviceArr);
		for (int i = 0; i < HEAD_SIZE; ++i) {
			printf("%lf  ", hostArr[i]);
		}
		printf("\n");
	}

	void getHeadAmp(Qureg qureg) {
		getDeviceArrHead(qureg.deviceStateVecList[0].real);
		//getDeviceArrHead(qureg.deviceStateVecList[0].imag);
		getDeviceArrHead(qureg.deviceStateVecList[1].real);
		//getDeviceArrHead(qureg.deviceStateVecList[0].imag);
	}
#endif // DEV

//hadmand
#ifdef DEV
	__global__ void statevec_hadamardKernel_localGPU(
		qreal *stateVecReal,
		qreal *stateVecImag,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock
	) {
		qreal  stateRealUp, stateRealLo, stateImagUp, stateImagLo;

		long long int thisTask = blockIdx.x*blockDim.x + threadIdx.x;
		if (thisTask >= numTasks) return;

		long long int thisBlock = thisTask / sizeHalfBlock;
		long long int indexUp = thisBlock * sizeBlock + thisTask % sizeHalfBlock;
		long long int indexLo = indexUp + sizeHalfBlock;


		stateRealUp = stateVecReal[indexUp];
		stateImagUp = stateVecImag[indexUp];

		stateRealLo = stateVecReal[indexLo];
		stateImagLo = stateVecImag[indexLo];

		stateVecReal[indexUp] = recRoot2 * (stateRealUp + stateRealLo);
		stateVecImag[indexUp] = recRoot2 * (stateImagUp + stateImagLo);

		stateVecReal[indexLo] = recRoot2 * (stateRealUp - stateRealLo);
		stateVecImag[indexLo] = recRoot2 * (stateImagUp - stateImagLo);
	}
	// 多GPU版本，支持访问外部（其他GPU）的数据
	__global__ void statevec_hadamardKernel_outerData(
		qreal *stateVecReal,
		qreal *stateVecImag,
		qreal *pairStateVecReal,
		qreal *pairStateVecImag,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock,
		const int isLoChunk
	) {

		qreal  stateRealUp, stateRealLo, stateImagUp, stateImagLo;

		long long int thisTask = blockIdx.x*blockDim.x + threadIdx.x;
		if (thisTask >= numTasks) return;

		if (isLoChunk==0) {
			stateRealUp = stateVecReal[thisTask];
			stateImagUp = stateVecImag[thisTask];

			stateRealLo = pairStateVecReal[thisTask];
			stateImagLo = pairStateVecImag[thisTask];

			stateVecReal[thisTask] = recRoot2 * (stateRealUp + stateRealLo);
			stateVecImag[thisTask] = recRoot2 * (stateImagUp + stateImagLo);
		}
		else {
			stateRealLo = stateVecReal[thisTask];
			stateImagLo = stateVecImag[thisTask];

			stateRealUp = pairStateVecReal[thisTask];
			stateImagUp = pairStateVecImag[thisTask];

			stateVecReal[thisTask] = recRoot2 * (stateRealUp - stateRealLo);
			stateVecImag[thisTask] = recRoot2 * (stateImagUp - stateImagLo);
		}

	}
	void statevec_hadamard_multiGPU(Qureg qureg, const int targetQubit) {

		const long long int sizeHalfBlock = 1LL << targetQubit;
		const long long int sizeBlock = 2LL * sizeHalfBlock;
		long long int CUDABlocks;
		int deviceID = qureg.chunkId;
		if (sizeBlock <= qureg.numAmpsPerChunk) {//data on local gpu is enough for blochsize
			const long long int numTasks = qureg.numAmpsPerChunk >> 1;
			CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk >> 1) / THREADS_PER_CUDA_BLOCK);

			statevec_hadamardKernel_localGPU << <
				CUDABlocks, THREADS_PER_CUDA_BLOCK
				>> > (qureg.deviceStateVec.real, qureg.deviceStateVec.imag, numTasks, sizeHalfBlock, sizeBlock);

		}
		else
		{//data on local gpu is not enough for blochsize
			int halfChunkPerBlock = sizeHalfBlock / qureg.numAmpsPerChunk;
			int chunkPerBlock = 2 * halfChunkPerBlock;
			int isLoChunk = ((deviceID % chunkPerBlock) >= halfChunkPerBlock);
			int outerChunkID;
			qreal* dstReal;
			qreal* dstImag;
			qreal* srcReal;
			qreal* srcImag;
			if (!isLoChunk) {
				outerChunkID = deviceID + halfChunkPerBlock;
			}
			//由于使用了cudaMemcpy，使用多个流
			if (!isLoChunk) {
				//使用多流将两个设备deviceID，outerChunkID的任务都完成，之后isLoChunk为True时不做任何工作
				long long int ampsPerStream = qureg.numAmpsPerChunk / USE_STREAM;
				CUDABlocks = ceil((qreal)(ampsPerStream / THREADS_PER_CUDA_BLOCK));
				for (int i = 0; i < USE_STREAM; i++) {
					setDevice(&qureg, deviceID);
					//prepare data for work on GPU deviceID
					dstReal = qureg.devicePairStateVec.real;
					dstImag = qureg.devicePairStateVec.imag;
					srcReal = qureg.deviceStateVecList[outerChunkID].real;
					srcImag = qureg.deviceStateVecList[outerChunkID].imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));

					//prepare data for work on GPU outerChunkID
					dstReal = qureg.devicePairStateVecList[outerChunkID].real;
					dstImag = qureg.devicePairStateVecList[outerChunkID].imag;
					srcReal = qureg.deviceStateVec.real;
					srcImag = qureg.deviceStateVec.imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));
					
					sycAllDevices(qureg);

					//allocate work on GPU deviceID
					statevec_hadamardKernel_outerData << <
						CUDABlocks, THREADS_PER_CUDA_BLOCK, 0,qureg.streamList[deviceID][i] >> > (
							qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 0);

					//allocate work on GPU outerChunkID
					setDevice(&qureg, outerChunkID);
					statevec_hadamardKernel_outerData << <
						CUDABlocks, THREADS_PER_CUDA_BLOCK, 0, qureg.streamList[outerChunkID][i] >> > (
							qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 1);
				}
			}
		}
	}
#endif // DEV

//CNOT
#ifdef DEV
	static __device__ int extractBit(int locationOfBitFromRight, long long int theEncodedNumber)
	{
		return (theEncodedNumber & (1LL << locationOfBitFromRight)) >> locationOfBitFromRight;
	}
	__global__ void statevec_controlledNotKernel_localGPU(
		Qureg qureg, const int controlQubit,
		const long long int numTasks, 
		const long long int sizeHalfBlock, 
		const long long int sizeBlock)
	{
		// store current state vector values in temp variables
		qreal   stateRealUp, stateImagUp;
		long long int thisBlock, indexUp, indexLo, globalIndexUp;
		qreal *stateVecReal = qureg.deviceStateVec.real;
		qreal *stateVecImag = qureg.deviceStateVec.imag;

		long long int thisTask = blockIdx.x*blockDim.x + threadIdx.x;
		if (thisTask >= numTasks) return;
		thisBlock = thisTask / sizeHalfBlock;
		indexUp = thisBlock * sizeBlock + thisTask % sizeHalfBlock;
		indexLo = indexUp + sizeHalfBlock;

		globalIndexUp = qureg.chunkId*qureg.numAmpsPerChunk + indexUp;
		int controlBit = extractBit(controlQubit, globalIndexUp);
		if (controlBit) {
			stateRealUp = stateVecReal[indexUp];
			stateImagUp = stateVecImag[indexUp];

			stateVecReal[indexUp] = stateVecReal[indexLo];
			stateVecImag[indexUp] = stateVecImag[indexLo];

			stateVecReal[indexLo] = stateRealUp;
			stateVecImag[indexLo] = stateImagUp;
		}
	}
	__global__ void statevec_controlledNotKernel_outerData(
		Qureg qureg, const int controlQubit,
		qreal *stateVecReal,
		qreal *stateVecImag,
		qreal *pairStateVecReal,
		qreal *pairStateVecImag,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock,
		const int isLoChunk
	) {
		qreal  stateRealUp, stateRealLo, stateImagUp, stateImagLo;

		long long int thisTask = blockIdx.x*blockDim.x + threadIdx.x;
		if (thisTask >= numTasks) return;
		long long int globalIndexUp = qureg.chunkId*qureg.numAmpsPerChunk + thisTask;
		int controlBit = extractBit(controlQubit, globalIndexUp);
		if (controlBit) {
			stateVecReal[thisTask] = pairStateVecReal[thisTask];
			stateVecImag[thisTask] = pairStateVecImag[thisTask];
		}
	}
	void statevec_controlledNot_multiGPU(Qureg qureg, const int controlQubit, const int targetQubit)
	{
		const long long int sizeHalfBlock = 1LL << targetQubit;
		const long long int sizeBlock = 2LL * sizeHalfBlock;
		int deviceID = qureg.chunkId;
		if (sizeBlock <= qureg.numAmpsPerChunk) {
			const long long int numTasks = qureg.numAmpsPerChunk >> 1;
			int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk >> 1) / THREADS_PER_CUDA_BLOCK);
			statevec_controlledNotKernel_localGPU << <CUDABlocks, THREADS_PER_CUDA_BLOCK >> > (
				qureg, controlQubit, numTasks, sizeHalfBlock, sizeBlock);
		}
		else {
			int halfChunkPerBlock = sizeHalfBlock / qureg.numAmpsPerChunk;
			int chunkPerBlock = 2 * halfChunkPerBlock;
			int isLoChunk = ((deviceID % chunkPerBlock) >= halfChunkPerBlock);
			int outerChunkID;
			qreal* dstReal;
			qreal* dstImag;
			qreal* srcReal;
			qreal* srcImag;
			if (isLoChunk) {
				outerChunkID = deviceID - halfChunkPerBlock;

				long long int ampsPerStream = qureg.numAmpsPerChunk / USE_STREAM;
				int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk / THREADS_PER_CUDA_BLOCK / USE_STREAM));
				
				for (int i = 0; i < USE_STREAM; i++) {
					setDevice(&qureg, deviceID);
					//prepare data for work on GPU deviceID
					dstReal = qureg.devicePairStateVec.real;
					dstImag = qureg.devicePairStateVec.imag;
					srcReal = qureg.deviceStateVecList[outerChunkID].real;
					srcImag = qureg.deviceStateVecList[outerChunkID].imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));

					//prepare data for work on GPU outerChunkID
					dstReal = qureg.devicePairStateVecList[outerChunkID].real;
					dstImag = qureg.devicePairStateVecList[outerChunkID].imag;
					srcReal = qureg.deviceStateVec.real;
					srcImag = qureg.deviceStateVec.imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));

					//both memcpy process should finish before kernel function is called
					sycAllDevices(qureg);

					//allocate work on GPU deviceID
					statevec_controlledNotKernel_outerData << <
						CUDABlocks, THREADS_PER_CUDA_BLOCK, 0, qureg.streamList[deviceID][i] >> > (
							qureg, controlQubit,
							qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 0);

					//allocate work on GPU outerChunkID
					setDevice(&qureg, outerChunkID);
					statevec_controlledNotKernel_outerData << <
						CUDABlocks, THREADS_PER_CUDA_BLOCK, 0, qureg.streamList[outerChunkID][i] >> > (
							qureg, controlQubit,
							qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 1);
				}
			}
		}
	}

#endif // DEV

//controlledCompactUnitary
#ifdef DEV
	//multiGpu implementation
	__global__ void statevec_compactUnitaryKernel_localGPU(
		Complex alpha, Complex beta,
		qreal *stateVecReal,
		qreal *stateVecImag,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock)
	{

		qreal stateRealUp, stateRealLo,
			stateImagUp, stateImagLo,
			betaReal, betaImag,
			alphaImag, alphaReal;

		long long int thisTask = blockIdx.x * blockDim.x + threadIdx.x;
		if (thisTask >= numTasks)
			return;

		betaReal = beta.real;
		betaImag = beta.imag;
		alphaReal = alpha.real;
		alphaImag = alpha.imag;

		long long int thisBlock = thisTask / sizeHalfBlock;
		long long int indexUp = thisBlock * sizeBlock + thisTask % sizeHalfBlock;
		long long int indexLo = indexUp + sizeHalfBlock;

		stateRealUp = stateVecReal[indexUp];
		stateImagUp = stateVecImag[indexUp];

		stateRealLo = stateVecReal[indexLo];
		stateImagLo = stateVecImag[indexLo];

		stateVecReal[indexUp] = alphaReal * stateRealUp - alphaImag * stateImagUp - betaReal * stateRealLo - betaImag * stateImagLo;
		stateVecImag[indexUp] = alphaReal * stateImagUp + alphaImag * stateRealUp - betaReal * stateImagLo + betaImag * stateRealLo;

		// state[indexLo] = beta  * state[indexUp] + conj(alpha) * state[indexLo]
		stateVecReal[indexLo] = betaReal * stateRealUp - betaImag * stateImagUp + alphaReal * stateRealLo + alphaImag * stateImagLo;
		stateVecImag[indexLo] = betaReal * stateImagUp + betaImag * stateRealUp + alphaReal * stateImagLo - alphaImag * stateRealLo;
	}

	__global__ void statevec_compactUnitaryKernel_outerData(
		Complex alpha, Complex beta,
		qreal *stateVecReal,
		qreal *stateVecImag,
		qreal *pairStateVecReal,
		qreal *pairStateVecImag,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock,
		const int isLoChunk)
	{
		qreal stateRealUp, stateRealLo,
			stateImagUp, stateImagLo,
			betaReal, betaImag,
			alphaImag, alphaReal;

		long long int thisTask = blockIdx.x * blockDim.x + threadIdx.x;
		if (thisTask >= numTasks)
			return;

		betaReal = beta.real;
		betaImag = beta.imag;
		alphaReal = alpha.real;
		alphaImag = alpha.imag;

		if (isLoChunk == 0)
		{
			stateRealUp = stateVecReal[thisTask];
			stateImagUp = stateVecImag[thisTask];

			stateRealLo = pairStateVecReal[thisTask];
			stateImagLo = pairStateVecImag[thisTask];

			stateVecReal[thisTask] = alphaReal * stateRealUp - alphaImag * stateImagUp - betaReal * stateRealLo - betaImag * stateImagLo;
			stateVecImag[thisTask] = alphaReal * stateImagUp + alphaImag * stateRealUp - betaReal * stateImagLo + betaImag * stateRealLo;
		}
		else
		{
			stateRealLo = stateVecReal[thisTask];
			stateImagLo = stateVecImag[thisTask];

			stateRealUp = pairStateVecReal[thisTask];
			stateImagUp = pairStateVecImag[thisTask];

			stateVecReal[thisTask] = betaReal * stateRealUp - betaImag * stateImagUp + alphaReal * stateRealLo + alphaImag * stateImagLo;
			stateVecImag[thisTask] = betaReal * stateImagUp + betaImag * stateRealUp + alphaReal * stateImagLo - alphaImag * stateRealLo;
		}
	}
	void statevec_compactUnitary_multiGPU(Qureg qureg, const int rotQubit, Complex alpha, Complex beta)
	{
		const long long int sizeHalfBlock = 1LL << rotQubit;
		const long long int sizeBlock = 2LL * sizeHalfBlock;
		int deviceID = qureg.chunkId;
		if (sizeBlock <= qureg.numAmpsPerChunk)
		{ //data on local gpu is enough for blochsize
			const long long int numTasks = qureg.numAmpsPerChunk >> 1;
			int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk >> 1) / THREADS_PER_CUDA_BLOCK);

			statevec_compactUnitaryKernel_localGPU << <
				CUDABlocks, THREADS_PER_CUDA_BLOCK >> > (alpha, beta, qureg.deviceStateVec.real,
					qureg.deviceStateVec.imag, numTasks, sizeHalfBlock, sizeBlock);
		}
		else
		{ //data on local gpu is not enough for blochsize
			int halfChunkPerBlock = sizeHalfBlock / qureg.numAmpsPerChunk;
			int chunkPerBlock = 2 * halfChunkPerBlock;
			int isLoChunk = ((deviceID % chunkPerBlock) >= halfChunkPerBlock); //numchunks 即为设备的数量
			int outerChunkID;
			qreal *dstReal;
			qreal *dstImag;
			qreal *srcReal;
			qreal *srcImag;

			if (!isLoChunk)
			{
				outerChunkID = deviceID + halfChunkPerBlock;

				//使用多流将两个设备deviceID，outerChunkID的任务都完成，之后isLoChunk为True时不做任何工作
				long long int ampsPerStream = qureg.numAmpsPerChunk / USE_STREAM;
				int CUDABlocks = ceil((qreal)(ampsPerStream / THREADS_PER_CUDA_BLOCK));
				for (int i = 0; i < USE_STREAM; i++)
				{
					setDevice(&qureg, deviceID);
					//prepare data for work on GPU deviceID
					dstReal = qureg.devicePairStateVec.real;
					dstImag = qureg.devicePairStateVec.imag;
					srcReal = qureg.deviceStateVecList[outerChunkID].real;
					srcImag = qureg.deviceStateVecList[outerChunkID].imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));

					//prepare data for work on GPU outerChunkID
					dstReal = qureg.devicePairStateVecList[outerChunkID].real;
					dstImag = qureg.devicePairStateVecList[outerChunkID].imag;
					srcReal = qureg.deviceStateVec.real;
					srcImag = qureg.deviceStateVec.imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));

					sycAllDevices(qureg);

					//allocate work on GPU deviceID
					statevec_compactUnitaryKernel_outerData << <
						CUDABlocks, THREADS_PER_CUDA_BLOCK, 0, qureg.streamList[deviceID][i] >> > (alpha, beta,
							qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 0);

					//allocate work on GPU outerChunkID
					setDevice(&qureg, outerChunkID);
					statevec_compactUnitaryKernel_outerData << <
						CUDABlocks, THREADS_PER_CUDA_BLOCK, 0, qureg.streamList[outerChunkID][i] >> > (alpha, beta,
							qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 1);
				}
			}
		}
	}
#endif // DEV

//controlledCompactUnitary
#ifdef DEV

	__global__ void statevec_controlledCompactUnitaryKernel_localGPU(
		Qureg qureg, const int controlQubit,
		Complex alpha, Complex beta,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock)
	{
		qreal stateRealUp, stateImagUp, stateRealLo, stateImagLo,
			alphaImag, alphaReal, betaReal, betaImag;
		long long int thisBlock, indexUp, indexLo, globalIndexUp;
		qreal *stateVecReal = qureg.deviceStateVec.real;
		qreal *stateVecImag = qureg.deviceStateVec.imag;

		betaReal = beta.real;
		betaImag = beta.imag;
		alphaReal = alpha.real;
		alphaImag = alpha.imag;

		long long int thisTask = blockIdx.x * blockDim.x + threadIdx.x;
		if (thisTask >= numTasks)
			return;
		thisBlock = thisTask / sizeHalfBlock;
		indexUp = thisBlock * sizeBlock + thisTask % sizeHalfBlock;
		indexLo = indexUp + sizeHalfBlock;

		globalIndexUp = qureg.chunkId * qureg.numAmpsPerChunk + indexUp;
		int controlBit = extractBit(controlQubit, globalIndexUp);

		if (controlBit)
		{
			// store current state vector values in temp variables
			stateRealUp = stateVecReal[indexUp];
			stateImagUp = stateVecImag[indexUp];

			stateRealLo = stateVecReal[indexLo];
			stateImagLo = stateVecImag[indexLo];

			// state[indexUp] = alpha * state[indexUp] - conj(beta)  * state[indexLo]
			stateVecReal[indexUp] = alphaReal * stateRealUp - alphaImag * stateImagUp - betaReal * stateRealLo - betaImag * stateImagLo;
			stateVecImag[indexUp] = alphaReal * stateImagUp + alphaImag * stateRealUp - betaReal * stateImagLo + betaImag * stateRealLo;

			// state[indexLo] = beta  * state[indexUp] + conj(alpha) * state[indexLo]
			stateVecReal[indexLo] = betaReal * stateRealUp - betaImag * stateImagUp + alphaReal * stateRealLo + alphaImag * stateImagLo;
			stateVecImag[indexLo] = betaReal * stateImagUp + betaImag * stateRealUp + alphaReal * stateImagLo - alphaImag * stateRealLo;
		}
	}
	__global__ void statevec_controlledCompactUnitaryKernel_outerData(
		Qureg qureg, const int controlQubit,
		Complex alpha, Complex beta,
		qreal *stateVecReal,
		qreal *stateVecImag,
		qreal *pairStateVecReal,
		qreal *pairStateVecImag,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock,
		const int isLoChunk)
	{

		qreal stateRealUp, stateRealLo,
			stateImagUp, stateImagLo,
			betaReal, betaImag,
			alphaReal, alphaImag;

		long long int thisTask = blockIdx.x * blockDim.x + threadIdx.x;
		if (thisTask >= numTasks)
			return;

		betaReal = beta.real;
		betaImag = beta.imag;
		alphaReal = alpha.real;
		alphaImag = alpha.imag;


		long long int globalIndexUp = qureg.chunkId * qureg.numAmpsPerChunk + thisTask;
		int controlBit = extractBit(controlQubit, globalIndexUp);
		if (controlBit)
		{
			if (isLoChunk == 0)
			{

				stateRealUp = stateVecReal[thisTask];
				stateImagUp = stateVecImag[thisTask];

				stateRealLo = pairStateVecReal[thisTask];
				stateImagLo = pairStateVecImag[thisTask];

				stateVecReal[thisTask] = alphaReal * stateRealUp - alphaImag * stateImagUp - betaReal * stateRealLo - betaImag * stateImagLo;

				stateVecImag[thisTask] = alphaReal * stateImagUp + alphaImag * stateRealUp - betaReal * stateImagLo + betaImag * stateRealLo;
			}
			else
			{
				stateRealLo = stateVecReal[thisTask];
				stateImagLo = stateVecImag[thisTask];

				stateRealUp = pairStateVecReal[thisTask];
				stateImagUp = pairStateVecImag[thisTask];

				stateVecReal[thisTask] = betaReal * stateRealUp - betaImag * stateImagUp + alphaReal * stateRealLo + alphaImag * stateImagLo;

				stateVecImag[thisTask] = betaReal * stateImagUp + betaImag * stateRealUp + alphaReal * stateImagLo - alphaImag * stateRealLo;
			}
		}
	}


	void statevec_controlledCompactUnitary_multiGPU(
		Qureg qureg, const int controlQubit, const int targetQubit,
		Complex alpha, Complex beta)
	{
		const long long int sizeHalfBlock = 1LL << targetQubit;
		const long long int sizeBlock = 2LL * sizeHalfBlock;
		int deviceID = qureg.chunkId;
		if (sizeBlock <= qureg.numAmpsPerChunk)
		{ //data on local gpu is enough for blochsize
			const long long int numTasks = qureg.numAmpsPerChunk >> 1;
			int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk >> 1) / THREADS_PER_CUDA_BLOCK);

			statevec_controlledCompactUnitaryKernel_localGPU << <
				CUDABlocks, THREADS_PER_CUDA_BLOCK >> > (qureg, controlQubit, alpha, beta,
					numTasks, sizeHalfBlock, sizeBlock);
		}
		else
		{ //data on local gpu is not enough for blochsize
			int halfChunkPerBlock = sizeHalfBlock / qureg.numAmpsPerChunk;
			int chunkPerBlock = 2 * halfChunkPerBlock;
			int isLoChunk = ((deviceID % chunkPerBlock) >= halfChunkPerBlock); //numchunks 即为设备的数量
			int outerChunkID;
			qreal *dstReal;
			qreal *dstImag;
			qreal *srcReal;
			qreal *srcImag;
			if (!isLoChunk)
			{
				outerChunkID = deviceID + halfChunkPerBlock;
			}
			if (!isLoChunk)
			{
				long long int ampsPerStream = qureg.numAmpsPerChunk / USE_STREAM;
				int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk / THREADS_PER_CUDA_BLOCK / USE_STREAM));

				for (int i = 0; i < USE_STREAM; i++)
				{
					setDevice(&qureg, deviceID);
					//prepare data for work on GPU deviceID
					dstReal = qureg.devicePairStateVec.real;
					dstImag = qureg.devicePairStateVec.imag;
					srcReal = qureg.deviceStateVecList[outerChunkID].real;
					srcImag = qureg.deviceStateVecList[outerChunkID].imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));

					//prepare data for work on GPU outerChunkID
					dstReal = qureg.devicePairStateVecList[outerChunkID].real;
					dstImag = qureg.devicePairStateVecList[outerChunkID].imag;
					srcReal = qureg.deviceStateVec.real;
					srcImag = qureg.deviceStateVec.imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));

					sycAllDevices(qureg);

					//allocate work on GPU deviceID
					statevec_controlledCompactUnitaryKernel_outerData << <CUDABlocks, THREADS_PER_CUDA_BLOCK, 0, qureg.streamList[deviceID][i] >> >
						(qureg, controlQubit, alpha, beta,
							qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 0);

					//allocate work on GPU outerChunkID
					setDevice(&qureg, outerChunkID);
					statevec_controlledCompactUnitaryKernel_outerData << <CUDABlocks, THREADS_PER_CUDA_BLOCK, 0, qureg.streamList[outerChunkID][i] >> >
						(qureg, controlQubit, alpha, beta,
							qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 1);
				}
			}
		}
	}
#endif // DEV

//phaseShiftByTerm
#ifdef DEV

	__global__ void statevec_phaseShiftByTermKernel_localGPU(
		qreal *stateVecReal,
		qreal *stateVecImag,
		qreal cosAngle, qreal sinAngle,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock
	) {
		qreal  stateRealUp, stateRealLo, stateImagUp, stateImagLo;

		long long int thisTask = blockIdx.x*blockDim.x + threadIdx.x;
		if (thisTask >= numTasks) return;

		long long int thisBlock = thisTask / sizeHalfBlock;
		long long int indexUp = thisBlock * sizeBlock + thisTask % sizeHalfBlock;
		long long int indexLo = indexUp + sizeHalfBlock;

		stateRealLo = stateVecReal[indexLo];
		stateImagLo = stateVecImag[indexLo];

		stateVecReal[indexLo] = cosAngle * stateRealLo - sinAngle * stateImagLo;
		stateVecImag[indexLo] = sinAngle * stateRealLo + cosAngle * stateImagLo;
	}
	__global__ void statevec_phaseShiftByTerm_outerData(
		qreal *stateVecReal,
		qreal *stateVecImag,
		qreal cosAngle, qreal sinAngle,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock)
	{

		qreal stateRealUp, stateRealLo,
			stateImagUp, stateImagLo,
			betaReal, betaImag,
			alphaReal, alphaImag;

		long long int thisTask = blockIdx.x * blockDim.x + threadIdx.x;
		if (thisTask >= numTasks)
			return;

		stateRealLo = stateVecReal[thisTask];
		stateImagLo = stateVecImag[thisTask];

		stateVecReal[thisTask] = cosAngle * stateRealLo - sinAngle * stateImagLo;
		stateVecImag[thisTask] = sinAngle * stateRealLo + cosAngle * stateImagLo;
		
	}
	
	void statevec_phaseShiftByTerm_multiGPU(Qureg qureg, const int targetQubit, Complex term) {

		qreal cosAngle = term.real;
		qreal sinAngle = term.imag;
		const long long int sizeHalfBlock = 1LL << targetQubit;
		const long long int sizeBlock = 2LL * sizeHalfBlock;
		int deviceID = qureg.chunkId;
		if (sizeBlock <= qureg.numAmpsPerChunk) {//data on local gpu is enough for blochsize
			const long long int numTasks = qureg.numAmpsPerChunk >> 1;
			int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk >> 1) / THREADS_PER_CUDA_BLOCK);

			statevec_phaseShiftByTermKernel_localGPU << <CUDABlocks, THREADS_PER_CUDA_BLOCK>> > (
				qureg.deviceStateVec.real, qureg.deviceStateVec.imag,
				cosAngle, sinAngle,
				numTasks, sizeHalfBlock, sizeBlock);

		}
		else
		{
			int halfChunkPerBlock = sizeHalfBlock / qureg.numAmpsPerChunk;
			int chunkPerBlock = 2 * halfChunkPerBlock;
			int isLoChunk = ((deviceID % chunkPerBlock) >= halfChunkPerBlock);
			if (isLoChunk) {
				int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk / THREADS_PER_CUDA_BLOCK));
				
				//only stateVec[indexLo] is referenced and modified
				statevec_phaseShiftByTerm_outerData << <CUDABlocks, THREADS_PER_CUDA_BLOCK>> > (
					qureg.deviceStateVec.real, qureg.deviceStateVec.imag,
					cosAngle, sinAngle,
					qureg.numAmpsPerChunk, sizeHalfBlock, sizeBlock);
				
			}
		}
	}
#endif // DEV

//pauliX
#ifdef DEV
	__global__ void statevec_pauliXKernel_localGPU(
		qreal *stateVecReal,
		qreal *stateVecImag,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock
	) {

		qreal  stateRealUp, stateRealLo, stateImagUp, stateImagLo;

		long long int thisTask = blockIdx.x*blockDim.x + threadIdx.x;
		if (thisTask >= numTasks) return;

		long long int thisBlock = thisTask / sizeHalfBlock;
		long long int indexUp = thisBlock * sizeBlock + thisTask % sizeHalfBlock;
		long long int indexLo = indexUp + sizeHalfBlock;

		// store current state vector values in temp variables
		stateRealUp = stateVecReal[indexUp];
		stateImagUp = stateVecImag[indexUp];

		stateVecReal[indexUp] = stateVecReal[indexLo];
		stateVecImag[indexUp] = stateVecImag[indexLo];

		stateVecReal[indexLo] = stateRealUp;
		stateVecImag[indexLo] = stateImagUp;
	}

	__global__ void statevec_pauliXKernel_outerData(
		qreal *stateVecReal,
		qreal *stateVecImag,
		qreal *pairStateVecReal,
		qreal *pairStateVecImag,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock,
		const int isLoChunk
	) {

		qreal  stateRealUp, stateRealLo, stateImagUp, stateImagLo;

		long long int thisTask = blockIdx.x*blockDim.x + threadIdx.x;
		if (thisTask >= numTasks) return;

		if (isLoChunk == 0) {//indexUp
			stateRealLo = pairStateVecReal[thisTask];
			stateImagLo = pairStateVecImag[thisTask];

			stateVecReal[thisTask] = stateRealLo;
			stateVecImag[thisTask] = stateImagLo;
		}
		else {//indexLo

			stateRealUp = pairStateVecReal[thisTask];
			stateImagUp = pairStateVecImag[thisTask];

			stateVecReal[thisTask] = stateRealUp;
			stateVecImag[thisTask] = stateImagUp;
		}
	}

	void statevec_pauliX_multiGPU(Qureg qureg, const int targetQubit)
	{
		const long long int sizeHalfBlock = 1LL << targetQubit;
		const long long int sizeBlock = 2LL * sizeHalfBlock;
		int deviceID = qureg.chunkId;
		if (sizeBlock <= qureg.numAmpsPerChunk)
		{ //data on local gpu is enough for blochsize
			const long long int numTasks = qureg.numAmpsPerChunk >> 1;
			int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk >> 1) / THREADS_PER_CUDA_BLOCK);
			statevec_pauliXKernel_localGPU << <CUDABlocks, THREADS_PER_CUDA_BLOCK >> >
				(qureg.deviceStateVec.real, qureg.deviceStateVec.imag, numTasks, sizeHalfBlock, sizeBlock);
		}
		else
		{ //data on local gpu is not enough for blochsize
			int halfChunkPerBlock = sizeHalfBlock / qureg.numAmpsPerChunk;
			int chunkPerBlock = 2 * halfChunkPerBlock;
			int isLoChunk = ((deviceID % chunkPerBlock) >= halfChunkPerBlock); //numchunks 即为设备的数量
			int outerChunkID;
			qreal *dstReal;
			qreal *dstImag;
			qreal *srcReal;
			qreal *srcImag;
			if (!isLoChunk)
			{
				outerChunkID = deviceID + halfChunkPerBlock;
				long long int ampsPerStream = qureg.numAmpsPerChunk / USE_STREAM;
				int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk / THREADS_PER_CUDA_BLOCK / USE_STREAM));

				for (int i = 0; i < USE_STREAM; i++)
				{
					setDevice(&qureg, deviceID);
					//prepare data for work on GPU deviceID
					dstReal = qureg.devicePairStateVec.real;
					dstImag = qureg.devicePairStateVec.imag;
					srcReal = qureg.deviceStateVecList[outerChunkID].real;
					srcImag = qureg.deviceStateVecList[outerChunkID].imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));

					//prepare data for work on GPU outerChunkID
					dstReal = qureg.devicePairStateVecList[outerChunkID].real;
					dstImag = qureg.devicePairStateVecList[outerChunkID].imag;
					srcReal = qureg.deviceStateVec.real;
					srcImag = qureg.deviceStateVec.imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));

					sycAllDevices(qureg);

					//allocate work on GPU deviceID
					statevec_pauliXKernel_outerData << <CUDABlocks, THREADS_PER_CUDA_BLOCK, 0, qureg.streamList[deviceID][i] >> >
						(qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 0);

					//allocate work on GPU outerChunkID
					setDevice(&qureg, outerChunkID);
					statevec_pauliXKernel_outerData << <CUDABlocks, THREADS_PER_CUDA_BLOCK, 0, qureg.streamList[outerChunkID][i] >> >
						(qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 1);
				}
			}
		}

	}

#endif // DEV

//pauliY
#ifdef DEV
	__global__ void statevec_pauliYKernel_localGPU(
		const int conjFac,
		qreal *stateVecReal,
		qreal *stateVecImag,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock
	) {

		qreal  stateRealUp, stateImagUp;

		long long int thisTask = blockIdx.x*blockDim.x + threadIdx.x;
		if (thisTask >= numTasks) return;

		long long int thisBlock = thisTask / sizeHalfBlock;
		long long int indexUp = thisBlock * sizeBlock + thisTask % sizeHalfBlock;
		long long int indexLo = indexUp + sizeHalfBlock;

		stateRealUp = stateVecReal[indexUp];
		stateImagUp = stateVecImag[indexUp];

		// update under +-{{0, -i}, {i, 0}}
		stateVecReal[indexUp] = conjFac * stateVecImag[indexLo];
		stateVecImag[indexUp] = conjFac * -stateVecReal[indexLo];
		stateVecReal[indexLo] = conjFac * -stateImagUp;
		stateVecImag[indexLo] = conjFac * stateRealUp;
	}
	__global__ void statevec_pauliYKernel_outerData(
		const int conjFac,
		qreal *stateVecReal,
		qreal *stateVecImag,
		qreal *pairStateVecReal,
		qreal *pairStateVecImag,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock,
		const int isLoChunk
	) {

		qreal  stateRealUp, stateRealLo, stateImagUp, stateImagLo;

		long long int thisTask = blockIdx.x*blockDim.x + threadIdx.x;
		if (thisTask >= numTasks) return;

		if (isLoChunk == 0) {//indexUp
			stateRealLo = pairStateVecReal[thisTask];
			stateImagLo = pairStateVecImag[thisTask];

			stateVecReal[thisTask] = conjFac * stateImagLo;
			stateVecImag[thisTask] = conjFac * -stateRealLo;
		}
		else {//indexLo

			stateRealUp = pairStateVecReal[thisTask];
			stateImagUp = pairStateVecImag[thisTask];

			stateVecReal[thisTask] = conjFac * -stateImagUp;
			stateVecImag[thisTask] = conjFac * stateRealUp;
		}
	}


	void statevec_pauliY_multiGPU(Qureg qureg, const int targetQubit)
	{
		const long long int sizeHalfBlock = 1LL << targetQubit;
		const long long int sizeBlock = 2LL * sizeHalfBlock;
		int deviceID = qureg.chunkId;
		if (sizeBlock <= qureg.numAmpsPerChunk)
		{ //data on local gpu is enough for blochsize
			const long long int numTasks = qureg.numAmpsPerChunk >> 1;
			int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk >> 1) / THREADS_PER_CUDA_BLOCK);
			statevec_pauliYKernel_localGPU << <CUDABlocks, THREADS_PER_CUDA_BLOCK >> >
				(1, qureg.deviceStateVec.real, qureg.deviceStateVec.imag, numTasks, sizeHalfBlock, sizeBlock);
		}
		else
		{ //data on local gpu is not enough for blochsize
			int halfChunkPerBlock = sizeHalfBlock / qureg.numAmpsPerChunk;
			int chunkPerBlock = 2 * halfChunkPerBlock;
			int isLoChunk = ((deviceID % chunkPerBlock) >= halfChunkPerBlock); //numchunks 即为设备的数量
			int outerChunkID;
			qreal *dstReal;
			qreal *dstImag;
			qreal *srcReal;
			qreal *srcImag;
			if (!isLoChunk)
			{
				outerChunkID = deviceID + halfChunkPerBlock;
				long long int ampsPerStream = qureg.numAmpsPerChunk / USE_STREAM;
				int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk / THREADS_PER_CUDA_BLOCK / USE_STREAM));

				for (int i = 0; i < USE_STREAM; i++)
				{
					setDevice(&qureg, deviceID);
					//prepare data for work on GPU deviceID
					dstReal = qureg.devicePairStateVec.real;
					dstImag = qureg.devicePairStateVec.imag;
					srcReal = qureg.deviceStateVecList[outerChunkID].real;
					srcImag = qureg.deviceStateVecList[outerChunkID].imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));

					//prepare data for work on GPU outerChunkID
					dstReal = qureg.devicePairStateVecList[outerChunkID].real;
					dstImag = qureg.devicePairStateVecList[outerChunkID].imag;
					srcReal = qureg.deviceStateVec.real;
					srcImag = qureg.deviceStateVec.imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));

					sycAllDevices(qureg);

					//allocate work on GPU deviceID
					statevec_pauliYKernel_outerData << <CUDABlocks, THREADS_PER_CUDA_BLOCK, 0, qureg.streamList[deviceID][i] >> >
						(1, qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 0);

					//allocate work on GPU outerChunkID
					setDevice(&qureg, outerChunkID);
					statevec_pauliYKernel_outerData << <CUDABlocks, THREADS_PER_CUDA_BLOCK, 0, qureg.streamList[outerChunkID][i] >> >
						(1, qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 1);
				}
			}
		}

	}


#endif // DEV

//controlledPauliY
#ifdef DEV
	__global__ void statevec_controlledPauliYKernel_outerData(
		Qureg qureg,
		const int controlQubit,
		const int conjFac,
		qreal *stateVecReal,
		qreal *stateVecImag,
		qreal *pairStateVecReal,
		qreal *pairStateVecImag,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock,
		const int isLoChunk)
	{
		qreal stateRealUp, stateRealLo,
			stateImagUp, stateImagLo;

		long long int thisTask = blockIdx.x * blockDim.x + threadIdx.x;
		if (thisTask >= numTasks)
			return;

		long long int globalIndexUp = qureg.chunkId * qureg.numAmpsPerChunk + thisTask;
		int controlBit = extractBit(controlQubit, globalIndexUp);
		if (controlBit)
		{
			if (isLoChunk == 0)
			{
				stateRealLo = pairStateVecReal[thisTask];
				stateImagLo = pairStateVecImag[thisTask];

				stateVecReal[thisTask] = conjFac * stateImagLo;
				stateVecImag[thisTask] = conjFac * -stateRealLo;
			}
			else
			{

				stateRealUp = pairStateVecReal[thisTask];
				stateImagUp = pairStateVecImag[thisTask];

				stateVecReal[thisTask] = conjFac * -stateImagUp;
				stateVecImag[thisTask] = conjFac * stateRealUp;
			}
		}

	}

	__global__ void statevec_controlledPauliYKernel_localGPU(
		Qureg qureg,
		const int controlQubit,
		const int conjFac,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock
	)
	{
		//qreal stateRealUp, stateImagUp, stateRealLo,stateImagLo;
		long long int thisBlock, indexUp, indexLo, globalIndexUp;
		int controlBit;

		qreal   stateRealUp, stateImagUp;

		qreal *stateVecReal = qureg.deviceStateVec.real;
		qreal *stateVecImag = qureg.deviceStateVec.imag;

		long long int thisTask = blockIdx.x*blockDim.x + threadIdx.x;
		if (thisTask >= numTasks) return;

		thisBlock = thisTask / sizeHalfBlock;
		indexUp = thisBlock * sizeBlock + thisTask % sizeHalfBlock;
		indexLo = indexUp + sizeHalfBlock;

		globalIndexUp = qureg.chunkId * qureg.numAmpsPerChunk + indexUp;
		controlBit = extractBit(controlQubit, globalIndexUp);
		if (controlBit) {

			stateRealUp = stateVecReal[indexUp];
			stateImagUp = stateVecImag[indexUp];

			// update under +-{{0, -i}, {i, 0}}
			stateVecReal[indexUp] = conjFac * stateVecImag[indexLo];
			stateVecImag[indexUp] = conjFac * -stateVecReal[indexLo];
			stateVecReal[indexLo] = conjFac * -stateImagUp;
			stateVecImag[indexLo] = conjFac * stateRealUp;
		}
	}

	void statevec_controlledPauliY_multiGPU(Qureg qureg, const int controlQubit, const int targetQubit)
	{
		const long long int sizeHalfBlock = 1LL << targetQubit;
		const long long int sizeBlock = 2LL * sizeHalfBlock;
		int deviceID = qureg.chunkId;
		if (sizeBlock <= qureg.numAmpsPerChunk)
		{ //data on local gpu is enough for blochsize
			const long long int numTasks = qureg.numAmpsPerChunk >> 1;
			int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk >> 1) / THREADS_PER_CUDA_BLOCK);
			statevec_controlledPauliYKernel_localGPU << <CUDABlocks, THREADS_PER_CUDA_BLOCK >> >
				(qureg, controlQubit, 1, numTasks, sizeHalfBlock, sizeBlock);
		}
		else
		{ //data on local gpu is not enough for blochsize
			int halfChunkPerBlock = sizeHalfBlock / qureg.numAmpsPerChunk;
			int chunkPerBlock = 2 * halfChunkPerBlock;
			int isLoChunk = ((deviceID % chunkPerBlock) >= halfChunkPerBlock); //numchunks 即为设备的数量
			int outerChunkID;
			qreal *dstReal;
			qreal *dstImag;
			qreal *srcReal;
			qreal *srcImag;
			if (!isLoChunk)
			{
				outerChunkID = deviceID + halfChunkPerBlock;
				long long int ampsPerStream = qureg.numAmpsPerChunk / USE_STREAM;
				int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk / THREADS_PER_CUDA_BLOCK / USE_STREAM));

				for (int i = 0; i < USE_STREAM; i++)
				{
					setDevice(&qureg, deviceID);
					//prepare data for work on GPU deviceID
					dstReal = qureg.devicePairStateVec.real;
					dstImag = qureg.devicePairStateVec.imag;
					srcReal = qureg.deviceStateVecList[outerChunkID].real;
					srcImag = qureg.deviceStateVecList[outerChunkID].imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[deviceID][i]));

					//prepare data for work on GPU outerChunkID
					dstReal = qureg.devicePairStateVecList[outerChunkID].real;
					dstImag = qureg.devicePairStateVecList[outerChunkID].imag;
					srcReal = qureg.deviceStateVec.real;
					srcImag = qureg.deviceStateVec.imag;
					CUDA_CALL(cudaMemcpyAsync(dstReal + i * ampsPerStream, srcReal + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));
					CUDA_CALL(cudaMemcpyAsync(dstImag + i * ampsPerStream, srcImag + i * ampsPerStream,
						ampsPerStream * sizeof(qreal), cudaMemcpyDefault, qureg.streamList[outerChunkID][i]));

					sycAllDevices(qureg);

					//allocate work on GPU deviceID
					statevec_controlledPauliYKernel_outerData << <CUDABlocks, THREADS_PER_CUDA_BLOCK, 0, qureg.streamList[deviceID][i] >> >
						(qureg, controlQubit, 1, qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 0);

					//allocate work on GPU outerChunkID
					setDevice(&qureg, outerChunkID);
					statevec_controlledPauliYKernel_outerData << <CUDABlocks, THREADS_PER_CUDA_BLOCK, 0, qureg.streamList[outerChunkID][i] >> >
						(qureg, controlQubit, 1, qureg.deviceStateVec.real + i * ampsPerStream, qureg.deviceStateVec.imag + i * ampsPerStream,
							qureg.devicePairStateVec.real + i * ampsPerStream, qureg.devicePairStateVec.imag + i * ampsPerStream,
							ampsPerStream, sizeHalfBlock, sizeBlock, 1);
				}
			}
		}
	}

#endif // DEV

#ifdef __cplusplus
}
#endif
