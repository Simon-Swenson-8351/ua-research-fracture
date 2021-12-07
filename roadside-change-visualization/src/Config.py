import wx

initialWindowSize = wx.Size(1280, 720)
mapFrameProportion = 0.75
roadFrameProportion = 0.67
dataFolder = 'data'
satelliteImageryFolder = 'satellite'
satelliteImageryFilename = 'satellite.jpg'
satteliteMetadataFilename = 'satellite.json'
roadFolder = 'road'
cameraImageFolder = 'camera-imagery'
minimapSize = wx.Size(64, 64)
nodeShape = 'circle'
roadPointWidth = 16
roadPointSelectedPenWidth = 2
roadLinkWidth = 6
roadLinkSelectedPenWidth = 2
scrollWheelScaleFactor = 1.3
selectedColor = wx.Colour(255, 255, 127)

# 0.0000925925925926 is about a ten meter resolution.
roadDrawingDegreesLatPerPixel = 0.0000925925925926