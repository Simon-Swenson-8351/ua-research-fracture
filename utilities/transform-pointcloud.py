# This script takes a obj point cloud and a json transformation matrix in 
# row-major form and outputs the transformed points.

import sys
import json
import numpy as np

if __name__ == '__main__':
    objFileName = sys.argv[1]
    transformationFileName = sys.argv[2]
    outputObjFileName = sys.argv[3]

    numPoints = 0
    with open(objFileName, 'r') as objFile:
        for line in objFile:
            print(line[:2])
            if line[:2] == 'v ':
                numPoints += 1
    initialPoints = np.zeros((4, numPoints))
    print(initialPoints.shape)
    with open(objFileName, 'r') as objFile:
        i = 0
        for line in objFile:
            spl = line.split(' ')
            initialPoints[0, i] = spl[1]
            initialPoints[1, i] = spl[2]
            initialPoints[2, i] = spl[3]
            initialPoints[3, i] = 1.0
            i += 1
    transformationMatList = None
    with open(transformationFileName, 'r') as transformationFile:
        transformationMatList = json.load(transformationFile)
    transformationMatrix = np.array(transformationMatList)

    transformedPoints = transformationMatrix @ initialPoints
    print(transformedPoints)
    with open(outputObjFileName, 'w') as outputObjFile:
        for i in range(transformedPoints.shape[1]):
            outputObjFile.write('v ' + str(transformedPoints[0, i]) + ' ' + str(transformedPoints[1, i]) + ' ' + str(transformedPoints[2, i]) + '\n')