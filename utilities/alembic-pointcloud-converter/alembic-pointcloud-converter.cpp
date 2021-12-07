#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreOgawa/All.h>
#include <Alembic/Abc/ErrorHandler.h>
#include <boost/algorithm/string.hpp>
#include <json/json.h>

// Converts a sfm.abc file output by the AliceVision SfM pipeline to an .obj 
// with point cloud data and a .json with camera data.
// arg 1 - Input (.abc) filename
// arg 2 - Camera output (.json) filename
// arg 3 - Point cloud output (.obj) filename

using namespace Alembic::Abc::ALEMBIC_VERSION_NS;
using namespace Alembic::AbcGeom::ALEMBIC_VERSION_NS;

std::string getImageName(std::string &cameraObjName, std::string &imageExtension) {
    // Example: camxform_00006_297819996_20180928_151556_008_00156_297819996
    // File name is: 20180928_151556_008
    // So we need to get the 4th to second-to-last 
    std::vector<std::string> tokens;
    boost::split(tokens, cameraObjName, [](char c){return c == '_';});
    std::string result = "";
    int i;
    for(i = 3; i < tokens.size() - 1; i++) {
        result += tokens[i];
        // If we're not the last one
        if(i != tokens.size() - 2) {
            result += "_";
        }
    }
    result += imageExtension;
    return result;
}

void printIndent(uint64_t indent) {
    uint64_t i;
    for(i = 0; i < indent; i++) {
        std::cout << "  ";
    }
}

void recursivePrintMetadata(IObject obj, uint64_t indent) {
    if(obj == NULL) return;
    printIndent(indent);
    std::cout << obj.getHeader().getName() << "\n";
    printIndent(indent);
    std::cout << obj.getHeader().getMetaData().serialize() << "\n";
    size_t i;
    for(i = 0; i < obj.getNumChildren(); i++) {
        recursivePrintMetadata(obj.getChild(i), indent + 1);
    }
}

int main(int argc, char *argv[]) {
    char *inputFileName = argv[1];
    char *cameraOutputFileName = argv[2];
    char *pointCloudOutputFileName = argv[3];
    std::string imageExtension(argv[4]);
    std::ofstream pointCloudOutputFile(pointCloudOutputFileName);
    // This is hard-coded to take an .abc file output by Alice Vision, SfM step.
    IArchive archive(Alembic::AbcCoreOgawa::ReadArchive(), inputFileName);
    recursivePrintMetadata(archive.getTop(), 0);
    // ABC.mvgRoot.mvgCameras
    IObject cameras = archive.getTop().getChild(0).getChild(0);
    // ABC.mvgRoot.mvgCloud.mvgPointCloud.particleShape1
    IObject pointsObj = archive.getTop().getChild(0).getChild(2).getChild(0).getChild(0);
    IPoints points(pointsObj, kWrapExisting);
    IPointsSchema::Sample pointsSample = points.getSchema().getValue();
    P3fArraySamplePtr pointsPositions = pointsSample.getPositions();

    // CAMERA OUTPUT
    Json::Value camerasJson(Json::arrayValue);
    std::ofstream cameraOutputFile(cameraOutputFileName);
    // cameraOutputFile << "[\n";
    size_t i;
    for(i = 0; i < cameras.getNumChildren(); i++) {
        IObject cameraObj = cameras.getChild(i);
        // Image name (without extension) for corresponding camera seems to be 
        // encoded in the camera name. Retrieve it here.
        // Example: camxform_00006_297819996_20180928_151556_008_00156_297819996
        // File name is: 20180928_151556_008
        std::string cameraObjName = cameraObj.getHeader().getName();
        std::string imageName = getImageName(cameraObjName, imageExtension);
        //std::cout << imageName << "\n";

        IXform cameraTransform(cameraObj, kWrapExisting);
        XformSample cameraTransformSample = cameraTransform.getSchema().getValue();
        M44d transformMatrix = cameraTransformSample.getMatrix();

        /*
        ICamera camera(cameraObj.getChild(0), kWrapExisting);
        CameraSample cameraSample = camera.getSchema().getValue();
        double swTop;
        double swBot;
        double swLeft;
        double swRight;
        Imath::Box3d childBounds = cameraSample.getChildBounds();
        cameraSample.getScreenWindow(swTop, swBot, swLeft, swRight);
        std::cout << "Screen Window: (" << swTop << ", " << swBot << ", " << swLeft << ", " << swRight << ")\n";
        std::cout << "Focal Length: " << cameraSample.getFocalLength() << "\n";
        std::cout << "Horizontal Aperture: " << cameraSample.getHorizontalAperture() << "\n";
        std::cout << "Horizontal Film Offset: " << cameraSample.getHorizontalFilmOffset() << "\n";
        std::cout << "Vertical Aperture: " << cameraSample.getVerticalAperture() << "\n";
        std::cout << "Vertical Film Offset: " << cameraSample.getVerticalFilmOffset() << "\n";
        std::cout << "Lens Squeeze Ratio: " << cameraSample.getLensSqueezeRatio() << "\n";
        std::cout << "Over Scan Left: " << cameraSample.getOverScanLeft() << "\n";
        std::cout << "Over Scan Right: " << cameraSample.getOverScanRight() << "\n";
        std::cout << "Over Scan Top: " << cameraSample.getOverScanTop() << "\n";
        std::cout << "Over Scan Bottom: " << cameraSample.getOverScanBottom() << "\n";
        std::cout << "F Stop: " << cameraSample.getFStop() << "\n";
        std::cout << "Focus Distance: " << cameraSample.getFocusDistance() << "\n";
        std::cout << "Shutter Open: " << cameraSample.getShutterOpen() << "\n";
        std::cout << "Shutter Close: " << cameraSample.getShutterClose() << "\n";
        std::cout << "Near Clipping Plane: " << cameraSample.getNearClippingPlane() << "\n";
        std::cout << "Far Clipping Plane: " << cameraSample.getFarClippingPlane() << "\n";
        std::cout << "Child Bounds: " << cameraSample.getChildBounds().min << ", " << cameraSample.getChildBounds().max << "\n";
        int j;
        for(j = 0; j < 16; j++) {
            std::cout << "Core Value (" << j << "): " << cameraSample.getCoreValue(j) << "\n";
        }
        std::cout << "FoV: " << cameraSample.getFieldOfView() << "\n";
        for(j = 0; j < cameraSample.getNumOps(); j++) {
            std::cout << j << "\n";
            //std::cout << "Op (" << j << "): " << cameraSample.getOp(j) << "\n";
        }
        std::cout << "Film Back Matrix: " << cameraSample.getFilmBackMatrix() << "\n";
        */

        Json::Value cameraJson(Json::objectValue);
        cameraJson["imageFileName"] = imageName;
        Json::Value cameraMatrixJson(Json::arrayValue);
        Json::Value cameraMatrixJsonRow1(Json::arrayValue);
        Json::Value cameraMatrixJsonRow2(Json::arrayValue);
        Json::Value cameraMatrixJsonRow3(Json::arrayValue);
        Json::Value cameraMatrixJsonRow4(Json::arrayValue);
        // Row major form
        cameraMatrixJsonRow1[0u] = transformMatrix[0][0];
        cameraMatrixJsonRow1[1u] = transformMatrix[1][0];
        cameraMatrixJsonRow1[2u] = transformMatrix[2][0];
        cameraMatrixJsonRow1[3u] = transformMatrix[3][0];

        cameraMatrixJsonRow2[0u] = transformMatrix[0][1];
        cameraMatrixJsonRow2[1u] = transformMatrix[1][1];
        cameraMatrixJsonRow2[2u] = transformMatrix[2][1];
        cameraMatrixJsonRow2[3u] = transformMatrix[3][1];

        cameraMatrixJsonRow3[0u] = transformMatrix[0][2];
        cameraMatrixJsonRow3[1u] = transformMatrix[1][2];
        cameraMatrixJsonRow3[2u] = transformMatrix[2][2];
        cameraMatrixJsonRow3[3u] = transformMatrix[3][2];

        cameraMatrixJsonRow4[0u] = transformMatrix[0][3];
        cameraMatrixJsonRow4[1u] = transformMatrix[1][3];
        cameraMatrixJsonRow4[2u] = transformMatrix[2][3];
        cameraMatrixJsonRow4[3u] = transformMatrix[3][3];

        cameraMatrixJson[0u] = cameraMatrixJsonRow1;
        cameraMatrixJson[1u] = cameraMatrixJsonRow2;
        cameraMatrixJson[2u] = cameraMatrixJsonRow3;
        cameraMatrixJson[3u] = cameraMatrixJsonRow4;

        cameraJson["cameraMatrix"] = cameraMatrixJson;

        camerasJson[(Json::ArrayIndex)i] = cameraJson;

        /*
        cameraOutputFile << "  {\n";
        cameraOutputFile << "    \"imageFileName\": \"" << imageName << "\",\n";
        cameraOutputFile << "    \"cameraMatrix\": [\n";
        cameraOutputFile << "      [" << transformMatrix[0][0] << ", " << transformMatrix[1][0] << ", " << transformMatrix[2][0] << ", " << transformMatrix[3][0] << "],\n";
        cameraOutputFile << "      [" << transformMatrix[0][1] << ", " << transformMatrix[1][1] << ", " << transformMatrix[2][1] << ", " << transformMatrix[3][1] << "],\n";
        cameraOutputFile << "      [" << transformMatrix[0][2] << ", " << transformMatrix[1][2] << ", " << transformMatrix[2][2] << ", " << transformMatrix[3][2] << "],\n";
        cameraOutputFile << "      [" << transformMatrix[0][3] << ", " << transformMatrix[1][3] << ", " << transformMatrix[2][3] << ", " << transformMatrix[3][3] << "]\n";
        cameraOutputFile << "    ]\n";
        cameraOutputFile << "  }";
        if(i != cameras.getNumChildren() - 1)
            cameraOutputFile << ",";
        cameraOutputFile << "\n";
        */
    }
    /*
    cameraOutputFile << "]\n";
    */

    cameraOutputFile << camerasJson << "\n";
    cameraOutputFile.close();

    // POINT OUTPUT. THIS SHOULD BE .OBJ FILE.
    for(i = 0; i < pointsPositions->size(); i++) {
        pointCloudOutputFile << "v " << (*pointsPositions)[i][0] << " " << (*pointsPositions)[i][1] << " " << (*pointsPositions)[i][2] << "\n";
    }
    pointCloudOutputFile.close();
    
    // Get the camera data
/*
    std::cout<<"traversing archive for elements\n";
    IObject obj=archive.getTop();
    unsigned int numChildren=obj.getNumChildren();
    std::cout<< "found "<<numChildren<<" children in file\n";

    for(int i=0; i<numChildren; ++i) {
        std::cout<<obj.getChildHeader(i).getFullName()<<"\n";
        IObject child(obj,obj.getChildHeader(i).getName());

        std::cout<<"Children "<<child.getNumChildren()<<"\n";
        const MetaData &md = child.getMetaData();
        std::cout<<md.serialize() <<"\n";

        for(int x=0; x<child.getNumChildren(); x++) {
            IObject child2(child,child.getChildHeader(x).getName());
            const MetaData &md2 = child2.getMetaData();
            if( IPolyMeshSchema::matches( md2 ) || ISubDSchema::matches( md2 )) {
                std::cout<<"Found a mesh "<<child2.getName()<<"\n"; 
            }
        }
    }
*/
}