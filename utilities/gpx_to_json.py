import sys
import datetime
import random
import json
import gpxpy
import gpxpy.gpx

random.seed(42)

inFilename = sys.argv[1]
outFilename = sys.argv[2]

outList = []

inGpx = None
with open(inFilename, 'r') as inFile:
    inGpx = gpxpy.parse(inFile)
    
for track in inGpx.tracks:
    for segment in track.segments:
        prevMaxVectorPerturbation = None
        for point in segment.points:

            # "discreteTimestampValues" demands an explanation. Essentially, 
            # it is an ordered list of (timestamps, vector values). The 
            # vector values are some representation of the surrounding 
            # environment's visual information. It's a way we can track 
            # change over time by comparing vector distances.
            curObj = {
                'lat': point.latitude,
                'longi': point.longitude,
                'discreteTimestampValues': []
            }

            ### BEGIN SHITTY SYNTHETIC DATA GENERATION CODE FOR discreteTimestampValues
            # We'll generate data on a monthly basis, starting from 
            # 10/1/2017 and ending at 10/1/2018
            vecLen = 10

            # Change maxVectorPerturbation for each point to simulate other 
            # areas changing more than others
            maxVectorPerturbation = None
            if prevMaxVectorPerturbation == None:
                maxVectorPerturbation = random.random()
            else:
                maxVectorPerturbation = prevMaxVectorPerturbation + ((random.random() - 0.5) / 10)
            prevVectorValue = None
            for mo in range(9, 21, 3):
                # Generate date
                yearOffset = mo // 12
                timestamp = {'year': 2017 + yearOffset, 'month': (mo % 12) + 1, 'day': 1}

                # Generate vector
                vectorValue = None
                if prevVectorValue == None:
                    vectorValue = []
                    for i in range(vecLen):
                        vectorValue.append(random.random() * 10)
                else:
                    vectorValue = prevVectorValue.copy()
                    for i in range(vecLen):
                        vectorValue[i] = vectorValue[i] + (random.random() * 2 - 1) * maxVectorPerturbation
                curObj['discreteTimestampValues'].append({'timestamp': timestamp, 'environmentVector': vectorValue})

                prevVectorValue = vectorValue
            prevMaxVectorPerturbation = maxVectorPerturbation
            ### END SHITTY SYNTHETIC DATA GENERATION CODE FOR discreteTimestampValues

            outList.append(curObj)

with open(outFilename, 'w') as outFile:
    print(json.dumps(outList, indent = '  ', separators=(',', ': ')))
    json.dump(outList, outFile, indent = '  ', separators=(',', ': '))