import sys
import json
import numpy as np
import math

# arg 1 - input camera transformation list
# arg 2 - output camera transformation list
# arg 3 - file to save the best fit transformation matrix

regularizationConvergence = 0.99
#initialRegularizationSearchDepth = 10
initialRegularizationConvergence = 0.95

def getLeastSquaredError(cameraMatches, curBestFit):
    err = 0.0
    for i in range(len(cameraMatches)):
        tmp = cameraMatches[i][1] - curBestFit @ cameraMatches[i][0]
        tmp = (np.dot(tmp, tmp) ** 2)
        err += tmp
    err /= (2 * len(cameraMatches))
    return err
                
# Gradient of \frac{1}{2} \sum_i | | y_i - M x_i | |^2 
# Basically converting this expression to code:
# \frac{d}{dM_{r,c}} = \sum_i (-x_{c, i})(y_{r, c} - \sum_j m_{r, j} x_{j, i})
def getGradient(cameraMatches, curBestFit):
    gradient = np.zeros((4, 4))
    for i in range(len(cameraMatches)):
        tmp = cameraMatches[i][1] - curBestFit @ cameraMatches[i][0]
        tmp = np.tile(tmp, (4, 1)).transpose()
        tmp = tmp * np.tile(-cameraMatches[i][0], (4, 1))
        gradient += tmp
    gradient /= len(cameraMatches)
    return gradient

def getBestFitMatrix(cameraMatches):
    bestFit = np.zeros((4, 4))
    prev_err = math.inf
    err = math.inf
    min_err_delta = 1.0e-10
    while True:
        bestFit -= getGradient(cameraMatches, bestFit)
        prev_err = err
        err = getLeastSquaredError(cameraMatches, bestFit)
        print('err = ' + str(err))
        if abs(prev_err - err) < min_err_delta:
            break
    print(bestFit)
    return bestFit

if __name__ == '__main__':
    cameraSet1Filename = sys.argv[1]
    cameraSet2Filename = sys.argv[2]
    outFilename = sys.argv[3]

    with open(cameraSet1Filename, 'r') as cameraSet1File:
        cameraSet1 = json.load(cameraSet1File)
    with open(cameraSet2Filename, 'r') as cameraSet2File:
        cameraSet2 = json.load(cameraSet2File)

    # Find matches touples
    cameraMatches = []
    for camera1 in cameraSet1:
        for camera2 in cameraSet2:
            if camera1['imageFileName'] == camera2['imageFileName']:
                cameraMatches.append((np.array(camera1['cameraMatrix'])[:, 3], np.array(camera2['cameraMatrix'])[:, 3]))
    # Below row is to test something. The cameras are basically coplanar, so we 
    # add another point (from the mesh) that is not coplanar to see if we get a
    # better result.
    # Update: Okay it works. So we need at least one non-coplanar pair to find 
    # an adequate match.
    #cameraMatches.append((np.array([0.34037, -0.44435, 1.18995, 1.0]), np.array([2.77132, -2.75071, 3.92789, 1.0])))

    # Note indexing is done row-major below
    # Trying to find transformation M which minimizes 1/2 * sum((O_i - M * I_i)^2)
    # (squared error)
    # We need to differentiate by M terms, then probably do gradient descent.
    # Note that we want to find matrix terms. Thus, each error term should be 
    # based on the term-wise output of O_i_{i,j} - (M * I_i)_{i,j}. Then, square 
    # each of those terms, add them all up, then take the derivative.
    # Expression for gradient ends up being something like this:
    # d/dM_{l,m} = sum_i sum_j (-I_i_{m,j})(O_{l,j} - sum_k M_{l,k} I_i_{k,j})
    bestFitMatrix = getBestFitMatrix(cameraMatches)

    with open(outFilename, 'w') as outFile:
        json.dump(bestFitMatrix.tolist(), outFile, indent = '  ', separators=(',', ': '))