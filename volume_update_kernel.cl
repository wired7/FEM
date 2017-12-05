

__kernel void update_volume(__global int* offsets, __global int* size, __global float* renderable, __global float* vertices, __constant float* cameraPos, __constant float* cameraDir, __constant float* separationDistance)
{
    // Get the index of the current element
    int i = get_global_id(0);
	
	int volumeIndex = offsets[i];
	int volumeSize = size[i];
	
	bool render = true;

	for(int j = 0; j < volumeSize; j++)
	{
		int vertexPosition = (volumeIndex + j) * 6;
		float distance = 0.0f;
		for(int k = 0; k < 3; k++)
		{
			distance += cameraDir[k] * (vertices[vertexPosition + k] - cameraPos[k]);
		}
		
		if(distance < separationDistance[0])
		{
			render = false;
			break;
		}
	}
	
	for(int j = 0; j < volumeSize; j++)
	{
		if(render)
		{
			renderable[volumeIndex + j] = 1.0f;
		}
		else
		{
			renderable[volumeIndex + j] = 0.0f;
		}
	}
}