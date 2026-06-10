#include "NAMProcessor.h"
#include "NAMController.h"
#include "NAMUIDs.h"
#include "public.sdk/source/main/pluginfactory.h"

using namespace Steinberg;

// Factory definition
BEGIN_FACTORY("Chopiou", "https://github.com/chopiou", "mailto:chopiou@example.com", Vst::kDefaultFactoryFlags)

// Processor
DEF_CLASS2(INLINE_UID_FROM_FUID(NAMProcessorUID),
           PClassInfo::kManyInstances,
           kVstAudioEffectClass,
           "NAM Loader",
           Vst::kDistributable,        // classFlags
           "Fx",                       // subCategories
           "1.0.0",                    // version
           kVstVersionString,          // sdkVersion
           NAMProcessor::createInstance)

// Controller
DEF_CLASS2(INLINE_UID_FROM_FUID(NAMControllerUID),
           PClassInfo::kManyInstances,
           kVstComponentControllerClass,
           "NAM Loader Controller",
           0,                          // classFlags
           "",                         // subCategories
           "1.0.0",                    // version
           kVstVersionString,          // sdkVersion
           NAMController::createInstance)

END_FACTORY

// Note: InitModule/DeinitModule are provided by the SDK's moduleinit.cpp