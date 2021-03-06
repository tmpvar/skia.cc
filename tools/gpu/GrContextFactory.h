/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextFactory_DEFINED
#define GrContextFactory_DEFINED

#include "GrContext.h"
#include "GrContextOptions.h"

#include "gl/GLTestContext.h"
#include "SkTArray.h"

struct GrVkBackendContext;

namespace sk_gpu_test {

struct ContextInfo {
    ContextInfo()
        : fGrContext(nullptr), fGLContext(nullptr) { }
    ContextInfo(GrContext* grContext, GLTestContext* glContext)
        : fGrContext(grContext), fGLContext(glContext) { }
    GrContext* fGrContext;
    GLTestContext* fGLContext; //! Valid until the factory destroys it via abandonContexts() or
                               //! destroyContexts(). Null if context is not based on OpenGL.
};

/**
 * This is a simple class that is useful in test apps that use different
 * GrContexts backed by different types of GL contexts. It manages creating the
 * GL context and a GrContext that uses it. The GL/Gr contexts persist until the
 * factory is destroyed (though the caller can always grab a ref on the returned
 * Gr and GL contexts to make them outlive the factory).
 */
class GrContextFactory : SkNoncopyable {
public:
    // The availability of context types is subject to platform and build configuration
    // restrictions.
    enum ContextType {
        kGL_ContextType,            //! OpenGL context.
        kGLES_ContextType,          //! OpenGL ES context.
        kANGLE_ContextType,         //! ANGLE on DirectX OpenGL ES context.
        kANGLE_GL_ContextType,      //! ANGLE on OpenGL OpenGL ES context.
        kCommandBuffer_ContextType, //! Chromium command buffer OpenGL ES context.
        kMESA_ContextType,          //! MESA OpenGL context
        kNullGL_ContextType,        //! Non-rendering OpenGL mock context.
        kDebugGL_ContextType,       //! Non-rendering, state verifying OpenGL context.
        kVulkan_ContextType,        //! Vulkan
        kLastContextType = kVulkan_ContextType
    };

    //! OpenGL or OpenGL ES context depending on the platform. To be removed.
    static const ContextType kNativeGL_ContextType;

    static const int kContextTypeCnt = kLastContextType + 1;

    /**
     * Options for GL context creation. For historical and testing reasons the options will default
     * to not using GL_NV_path_rendering extension  even when the driver supports it.
     */
    enum ContextOptions {
        kNone_ContextOptions                = 0x0,
        kEnableNVPR_ContextOptions          = 0x1,
        kRequireSRGBSupport_ContextOptions  = 0x2,
    };

    static bool IsRenderingContext(ContextType type) {
        switch (type) {
            case kNullGL_ContextType:
            case kDebugGL_ContextType:
                return false;
            default:
                return true;
        }
    }

    static GrBackend ContextTypeBackend(ContextType type) {
        switch (type) {
            case kVulkan_ContextType:
                return kVulkan_GrBackend;
            default:
                return kOpenGL_GrBackend;
        }
    }

    static const char* ContextTypeName(ContextType type) {
        switch (type) {
            case kGL_ContextType:
                return "gl";
            case kGLES_ContextType:
                return "gles";
            case kANGLE_ContextType:
                return "angle";
            case kANGLE_GL_ContextType:
                return "angle-gl";
            case kCommandBuffer_ContextType:
                return "commandbuffer";
            case kMESA_ContextType:
                return "mesa";
            case kNullGL_ContextType:
                return "nullgl";
            case kDebugGL_ContextType:
                return "debuggl";
            case kVulkan_ContextType:
                return "vulkan";
        }
    }

    explicit GrContextFactory(const GrContextOptions& opts);
    GrContextFactory();

    ~GrContextFactory();

    void destroyContexts();
    void abandonContexts();
    void releaseResourcesAndAbandonContexts();

    /**
     * Get a context initialized with a type of GL context. It also makes the GL context current.
     */
    ContextInfo getContextInfo(ContextType type,
                               ContextOptions options = kNone_ContextOptions);
    /**
     * Get a GrContext initialized with a type of GL context. It also makes the GL context current.
     */
    GrContext* get(ContextType type, ContextOptions options = kNone_ContextOptions) {
        return this->getContextInfo(type, options).fGrContext;
    }
    const GrContextOptions& getGlobalOptions() const { return fGlobalOptions; }

private:
    struct Context {
        ContextType     fType;
        ContextOptions  fOptions;
        GLTestContext*  fGLContext; //  null if non-GL
        GrContext*      fGrContext;
        bool            fAbandoned;
    };
    SkTArray<Context, true>         fContexts;
    SkAutoTDelete<GLTestContext>    fSentinelGLContext;
    const GrContextOptions          fGlobalOptions;
};
}  // namespace sk_gpu_test
#endif
