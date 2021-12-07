import wx
import math
import numpy

def scaleIntSizePreserveAspectRatio(childObjectSize, parentObjectSize):
    scaleFactor = min(parentObjectSize.GetWidth()/childObjectSize.GetWidth(), parentObjectSize.GetHeight()/childObjectSize.GetHeight())
    return wx.Size(int(childObjectSize.GetWidth() * scaleFactor), int(childObjectSize.GetHeight() * scaleFactor))

def growAndCenterImage(image, newSize):
    image.Resize(
        newSize,
        wx.Point(
            (newSize.GetWidth() - image.GetWidth()) // 2,
            (newSize.GetHeight() - image.GetHeight()) // 2
        ),
        0,
        0,
        0
    )

def getLongLatRatio(lat):
    return math.cos(math.radians(lat))

def getMaxRoadDiff(roads):
    maxDiff = 0
    for roadName, road in roads.items():
        for roadLocation in road:
            for environmentVectorRecordA in roadLocation['discreteTimestampValues']:
                for environmentVectorRecordB in roadLocation['discreteTimestampValues']:
                    environmentVectorA = numpy.asarray(environmentVectorRecordA['environmentVector'])
                    environmentVectorB = numpy.asarray(environmentVectorRecordB['environmentVector'])
                    maxDiff = max(maxDiff, numpy.linalg.norm(environmentVectorA - environmentVectorB))
    return maxDiff

def intToColor(x):
    return wx.Colour((x >> 16) & 255, (x >> 8) & 255, x & 255)

def colorToInt(x):
    return (x.Red() << 16) + (x.Green() << 8) + x.Blue()

class AffineScaler:

    def __init__(self, x1, x2, y1, y2):

        self.m = (y1 - y2) / (x1 - x2)
        self.b = y1 - self.m * x1

    def transform(self, x):
        return self.m * x + self.b

    def inverse(self, y):
        return (y - self.b) / self.m