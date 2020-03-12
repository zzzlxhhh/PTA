
__device__ void reduceBlock2(qreal *arrayIn, qreal *reducedArray, int length) {
	int idx = threadIdx.x;

	// calculate lane index and warp index
	int laneIdx = threadIdx.x % warpSize;
	int warpIdx = threadIdx.x / warpSize;

	// blcok-wide warp reduce
	qreal localSum = warpReduce(arrayIn[idx]);

	// save warp sum to shared memory
	if (laneIdx == 0) arrayIn[warpIdx] = localSum;

	// block synchronization
	__syncthreads();

	//int size = ;
	// last warp reduce
	if (threadIdx.x < warpSize) localSum = (threadIdx.x < ((blockDim.x + 32 - 1) / 32)) ?
		arrayIn[laneIdx] : 0;

	if (warpIdx == 0)
		localSum = warpReduce(localSum);

	if (threadIdx.x == 0)
		reducedArray[blockIdx.x] = localSum;
}

__device__ void reduceBlock1(qreal *arrayIn, qreal *reducedArray, int length) {
	int i, l, r;
	int threadMax, maxDepth;
	threadMax = length / 2;
	maxDepth = log2Int(length / 2);

	for (i = 0; i < maxDepth + 1; i++) {
		if (threadIdx.x < threadMax) {
			l = threadIdx.x;
			r = l + threadMax;
			arrayIn[l] = arrayIn[r] + arrayIn[l];
		}
		threadMax = threadMax >> 1;
		__syncthreads(); // optimise -- use warp shuffle instead
	}

	if (threadIdx.x == 0) reducedArray[blockIdx.x] = arrayIn[0];
}


__device__ void reduceBlock(qreal *arrayIn, qreal *reducedArray, int length){
	unsigned int tid = threadIdx.x;

	//if (length >= 2048 && tid < 1024) arrayIn[tid] += arrayIn[tid + 1024];
	//__syncthreads();
	//if (length >= 1024 && tid < 512) arrayIn[tid] += arrayIn[tid + 512];
	//__syncthreads();
	if (length >= 512 && tid < 256) arrayIn[tid] += arrayIn[tid + 256];
	__syncthreads();
	if (length >= 256 && tid < 128) arrayIn[tid] += arrayIn[tid + 128];
	__syncthreads();
	if (length >= 128 && tid < 64) arrayIn[tid] += arrayIn[tid + 64];
	__syncthreads();
	if (length >= 64 && tid < 32) arrayIn[tid] += arrayIn[tid + 32];
	__syncthreads();

	if (length < 32)
	{
		reduceBlock1(arrayIn, reducedArray, length);
		return;
	}
	else
	{
		qreal localSum = arrayIn[tid];
		localSum += __shfl_xor(localSum, 16);
		localSum += __shfl_xor(localSum, 8);
		localSum += __shfl_xor(localSum, 4);
		localSum += __shfl_xor(localSum, 2);
		localSum += __shfl_xor(localSum, 1);

		if (tid == 0) reducedArray[blockIdx.x] = localSum;
	}

}


__global__ void copySharedReduceBlock(qreal*arrayIn, qreal *reducedArray, int length){
    extern __shared__ qreal tempReductionArray[];
    int blockOffset = blockIdx.x*length;
    tempReductionArray[threadIdx.x] = arrayIn[blockOffset + threadIdx.x];
   // tempReductionArray[threadIdx.x*2+1] = arrayIn[blockOffset + threadIdx.x*2+1];
    __syncthreads();
    reduceBlock(tempReductionArray, reducedArray, length);
}