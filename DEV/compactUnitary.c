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

			stateVecReal[thisTask] = alphaReal * stateRealUp - alphaImag * stateImagUp
				- betaReal * stateRealLo - betaImag * stateImagLo;
			stateVecImag[thisTask] = alphaReal * stateImagUp + alphaImag * stateRealUp
				- betaReal * stateImagLo + betaImag * stateRealLo;
		}
		else
		{
			stateRealLo = stateVecReal[thisTask];
			stateImagLo = stateVecImag[thisTask];

			stateRealUp = pairStateVecReal[thisTask];
			stateImagUp = pairStateVecImag[thisTask];

			stateVecReal[thisTask] = betaReal * stateRealUp - betaImag * stateImagUp
				+ alphaReal * stateRealLo + alphaImag * stateImagLo;
			stateVecImag[thisTask] = betaReal * stateImagUp + betaImag * stateRealUp
				+ alphaReal * stateImagLo - alphaImag * stateRealLo;
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
	static __device__ int extractBit(int locationOfBitFromRight, long long int theEncodedNumber)
	{
		return (theEncodedNumber & (1LL << locationOfBitFromRight)) >> locationOfBitFromRight;
	}

	__global__ void statevec_controlledCompactUnitaryKernel_localGPU(
		Qureg qureg, const int controlQubit,
		Complex alpha, Complex beta,
		const long long int numTasks,
		const long long int sizeHalfBlock,
		const long long int sizeBlock)
	{
		qreal stateRealUp, stateImagUp, stateRealLo,stateImagLo,
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
			int CUDABlocks = ceil((qreal)(qureg.numAmpsPerChunk) / THREADS_PER_CUDA_BLOCK);

			statevec_controlledCompactUnitaryKernel_localGPU << <
				CUDABlocks, THREADS_PER_CUDA_BLOCK >> > (qureg, controlQubit, alpha, beta, 
					qureg.deviceStateVec.real, qureg.deviceStateVec.imag, numTasks, sizeHalfBlock, sizeBlock);
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