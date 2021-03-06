//
// cint script to generate libraries
// Declaire namespace & classes you defined
// #pragma statement: order matters! Google it ;)
//

#ifdef __CINT__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class larlite::ExecuteCompression+;
#pragma link C++ class larlite::MCStudyCompression+;
#pragma link C++ class larlite::HitMaker+;
#pragma link C++ class larlite::HitViewer+;
#pragma link C++ class larlite::WFViewer+;
//ADD_NEW_CLASS ... do not change this line

#pragma link C++ namespace compress+;

#pragma link C++ class compress::CompressionAlgoBase+;
#pragma link C++ class compress::CompressionAlgoTwoThresh+;
#pragma link C++ class compress::CompressionAlgoOneThresh+;
#pragma link C++ class compress::CompressionAlgosncompress+;
#pragma link C++ class compress::ViewCompression+;
#pragma link C++ class compress::CompressionStudyBase+;
#pragma link C++ class compress::CompressionStudyHits+;
#pragma link C++ class compress::CompressionStudyBaseline+;
#pragma link C++ class compress::CompressionStudyIDEs+;

//ADD_COMP_CLASS ... do not change this line

#endif




