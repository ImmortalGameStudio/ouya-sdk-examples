/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>
#include <map>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/bitmap.h>
#include <android/log.h>
#include <android/sensor.h>
#include "../native_app_glue/android_native_app_glue.h"

#include "Activity.h"
#include "AssetManager.h"
#include "Bitmap.h"
#include "BitmapFactory.h"
#include "ClassLoader.h"
#include "Context.h"
#include "GLUtils.h"
#include "InputStream.h"
#include "OuyaController.h"
#include "String.h"
#include "System.h"

#define LOG_TAG "VirtualControllerNative"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

using namespace android_app_Activity;
using namespace android_content_Context;
using namespace android_content_res_AssetManager;
using namespace android_opengl_GLUtils;
using namespace android_graphics_Bitmap;
using namespace android_graphics_BitmapFactory;
using namespace java_io_InputStream;
using namespace java_lang_ClassLoader;
using namespace java_lang_String;
using namespace java_lang_System;
using namespace tv_ouya_console_api_OuyaController;

/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env;
	if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
		return -1;
	}

	LOGI("***************");
	LOGI("***************");
	LOGI("*****JNI*******");
	LOGI("******ONLOAD***");
	LOGI("***************");
	LOGI("***************");

	// Get jclass with env->FindClass.
	// Register methods with env->RegisterNatives.

	return JNI_VERSION_1_6;
}

jint RegisterClasses(ANativeActivity* activity)
{
	JNIEnv* env = activity->env;

	JavaVM* vm = activity->vm;
	vm->AttachCurrentThread(&env, NULL);

	jclass activityClass = env->GetObjectClass(activity->clazz);
	if (!activityClass)
	{
		LOGE("Failed to find activity class");
		return JNI_ERR;
	}
	else
	{
		LOGI("Found activity class");
	}

	// load the system first
	if (JNI_ERR == System::InitJNI(env))
	{
		return JNI_ERR;
	}

	// load the class loader second
	if (JNI_ERR == ClassLoader::InitJNI(env))
	{
		return JNI_ERR;
	}

	// get JNI_Onload to fire
	System::loadLibrary("ouya-sdk");

	if (JNI_ERR == Activity::InitJNI(env))
	{
		return JNI_ERR;
	}

	if (JNI_ERR == AssetManager::InitJNI(env))
	{
		return JNI_ERR;
	}

	if (JNI_ERR == Bitmap::InitJNI(env))
	{
		return JNI_ERR;
	}

	if (JNI_ERR == BitmapFactory::InitJNI(env))
	{
		return JNI_ERR;
	}

	if (JNI_ERR == BitmapFactory::Options::InitJNI(env))
	{
		return JNI_ERR;
	}

	if (JNI_ERR == Context::InitJNI(env))
	{
		return JNI_ERR;
	}

	if (JNI_ERR == GLUtils::InitJNI(env))
	{
		return JNI_ERR;
	}

	if (JNI_ERR == InputStream::InitJNI(env))
	{
		return JNI_ERR;
	}

	if (JNI_ERR == OuyaController::InitJNI(env))
	{
		return JNI_ERR;
	}

	if (JNI_ERR == String::InitJNI(env))
	{
		return JNI_ERR;
	}

	return JNI_VERSION_1_6;
}

const int textureCount = 17;
static GLuint textures[textureCount];

int LoadTexture(JNIEnv* env, AssetManager& assetManager, const BitmapFactory::Options& options, const char* texturePath, int textureId)
{
	String strController = String(texturePath);
	InputStream stream = assetManager.open(strController);
	Bitmap bitmap = BitmapFactory::decodeStream(stream, 0, options);
	stream.close();
	int width = bitmap.getWidth();
	int height = bitmap.getHeight();
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Loaded %s bitmap width=%d height=%d", texturePath, width, height);

	AndroidBitmapInfo info = AndroidBitmapInfo();
	AndroidBitmap_getInfo(env, bitmap.GetInstance(), &info);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "info.width=%d", info.width);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "info.height=%d", info.height);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "info.stride=%d", info.stride);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "info.format=%d", info.format);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "info.flags=%d", info.flags);

	if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Bitmap format is not RGBA_8888! %s", texturePath);
		return -1;
	}

	glBindTexture(GL_TEXTURE_2D, textures[textureId]);
	__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "glBindTexture=%d", textures[textureId]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	android_opengl_GLUtils::GLUtils::texImage2D(GL_TEXTURE_2D, 0, bitmap.GetInstance(), 0);

	bitmap.recycle();

	return textureId;
}

int g_controller = 0;
int g_buttonA = 0;
int g_buttonO = 0;
int g_buttonU = 0;
int g_buttonY = 0;
int g_dpadDown = 0;
int g_dpadLeft = 0;
int g_dpadRight = 0;
int g_dpadUp = 0;
int g_leftBumper = 0;
int g_leftTrigger = 0;
int g_leftStickInactive = 0;
int g_leftStickActive = 0;
int g_rightBumper = 0;
int g_rightTrigger = 0;
int g_rightStickInactive = 0;
int g_rightStickActive = 0;

void LoadBitmaps(JavaVM* vm, JNIEnv* env, jobject objActivity)
{
	vm->AttachCurrentThread(&env, NULL);

	BitmapFactory::Options options = BitmapFactory::Options();
	options.set_inScaled(false); // No pre-scaling
	Activity activity = Activity(objActivity);
	Context context = activity.getApplicationContext();
	AssetManager assetManager = context.getAssets();

	String path = String("");
	std::vector<std::string> files = assetManager.list(path);
	for (int index = 0; index < files.size(); ++index)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, files[index].c_str());
	}

	glGenTextures(textureCount, &textures[0]);

	int textureId = 0;
	g_controller = LoadTexture(env, assetManager, options, "controller.png", textureId);
	g_buttonA = LoadTexture(env, assetManager, options, "a.png", ++textureId);
	g_dpadDown = LoadTexture(env, assetManager, options, "dpad_down.png", ++textureId);
	g_dpadLeft = LoadTexture(env, assetManager, options, "dpad_left.png", ++textureId);
	g_dpadRight = LoadTexture(env, assetManager, options, "dpad_right.png", ++textureId);
	g_dpadUp = LoadTexture(env, assetManager, options, "dpad_up.png", ++textureId);
	g_leftBumper = LoadTexture(env, assetManager, options, "lb.png", ++textureId);
	g_leftTrigger = LoadTexture(env, assetManager, options, "lt.png", ++textureId);
	g_leftStickInactive = LoadTexture(env, assetManager, options, "l_stick.png", ++textureId);
	g_buttonO = LoadTexture(env, assetManager, options, "o.png", ++textureId);
	g_rightBumper = LoadTexture(env, assetManager, options, "rb.png", ++textureId);
	g_rightTrigger = LoadTexture(env, assetManager, options, "rt.png", ++textureId);
	g_rightStickInactive = LoadTexture(env, assetManager, options, "r_stick.png", ++textureId);
	g_leftStickActive = LoadTexture(env, assetManager, options, "thumbl.png", ++textureId);
	g_rightStickActive = LoadTexture(env, assetManager, options, "thumbr.png", ++textureId);
	g_buttonU = LoadTexture(env, assetManager, options, "u.png", ++textureId);
	g_buttonY = LoadTexture(env, assetManager, options, "y.png", ++textureId);

	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Loaded %d textures", textureId + 1);
}

static unsigned int vbo[3];
static float positions[12] = { 0.1, -0.1, 0.0, 0.1, 0.1, 0.0, -0.1, -0.1, 0.0, -0.1, 0.1, 0.0 };
static float textureCoords[8] = { 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
static short indices[4] = { 0, 1, 2, 3 };

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize OpenGL ES and EGL
	if (JNI_ERR == RegisterClasses(engine->app->activity))
	{
		return JNI_ERR;
	}

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;

    // Initialize GL state.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

	LoadBitmaps(engine->app->activity->vm, engine->app->activity->env, engine->app->activity->clazz);

	glGenBuffers(3, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 12, positions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 8, textureCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 4, indices, GL_STATIC_DRAW);

    return 0;
}

static void DrawTexture(int textureId)
{
	glBindTexture(GL_TEXTURE_2D, textures[textureId]);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }

	glClearColor(0, 0, 0, 1);

	glClear(GL_COLOR_BUFFER_BIT);

	// model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glRotatef(90, 0, 0, 1);
	glScalef(4, 4, 4);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// draw the model
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	DrawTexture(g_controller);
	DrawTexture(g_buttonA);
	DrawTexture(g_dpadDown);
	DrawTexture(g_dpadLeft);
	DrawTexture(g_dpadRight);
	DrawTexture(g_dpadUp);
	DrawTexture(g_leftBumper);
	DrawTexture(g_leftTrigger);
	DrawTexture(g_leftStickInactive);
	DrawTexture(g_buttonO);
	DrawTexture(g_rightBumper);
	DrawTexture(g_rightTrigger);
	DrawTexture(g_rightStickInactive);
	DrawTexture(g_leftStickActive);
	DrawTexture(g_rightStickActive);
	DrawTexture(g_buttonU);
	DrawTexture(g_buttonY);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

static void debugOuyaMotionEvent(AInputEvent* motionEvent)
{
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "OuyaController.AXIS_LS_X value=%f", AMotionEvent_getAxisValue(motionEvent, OuyaController::AXIS_LS_X(), 0));
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "OuyaController.AXIS_LS_Y value==%f", AMotionEvent_getAxisValue(motionEvent, OuyaController::AXIS_LS_Y(), 0));
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "OuyaController.AXIS_RS_X value==%f", AMotionEvent_getAxisValue(motionEvent, OuyaController::AXIS_RS_X(), 0));
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "OuyaController.AXIS_RS_Y value==%f", AMotionEvent_getAxisValue(motionEvent, OuyaController::AXIS_RS_Y(), 0));
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "OuyaController.AXIS_L2 value==%f", AMotionEvent_getAxisValue(motionEvent, OuyaController::AXIS_L2(), 0));
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "OuyaController.AXIS_R2 value==%f", AMotionEvent_getAxisValue(motionEvent, OuyaController::AXIS_R2(), 0));
}

static void debugMotionEvent(AInputEvent* motionEvent)
{
	std::map<int, const char*> names;
	names[AMOTION_EVENT_AXIS_X] = "AXIS_X";
	names[AMOTION_EVENT_AXIS_Y] = "AXIS_Y";
	names[AMOTION_EVENT_AXIS_PRESSURE] = "AXIS_PRESSURE";
	names[AMOTION_EVENT_AXIS_SIZE] = "AXIS_SIZE";
	names[AMOTION_EVENT_AXIS_TOUCH_MAJOR] = "AXIS_TOUCH_MAJOR";
	names[AMOTION_EVENT_AXIS_TOUCH_MINOR] = "AXIS_TOUCH_MINOR";
	names[AMOTION_EVENT_AXIS_TOOL_MAJOR] = "AXIS_TOOL_MAJOR";
	names[AMOTION_EVENT_AXIS_TOOL_MINOR] = "AXIS_TOOL_MINOR";
	names[AMOTION_EVENT_AXIS_ORIENTATION] = "AXIS_ORIENTATION";
	names[AMOTION_EVENT_AXIS_VSCROLL] = "AXIS_VSCROLL";
	names[AMOTION_EVENT_AXIS_HSCROLL] = "AXIS_HSCROLL";
	names[AMOTION_EVENT_AXIS_Z] = "AXIS_Z";
	names[AMOTION_EVENT_AXIS_RX] = "AXIS_RX";
	names[AMOTION_EVENT_AXIS_RY] = "AXIS_RY";
	names[AMOTION_EVENT_AXIS_RZ] = "AXIS_RZ";
	names[AMOTION_EVENT_AXIS_HAT_X] = "AXIS_HAT_X";
	names[AMOTION_EVENT_AXIS_HAT_Y] = "AXIS_HAT_Y";
	names[AMOTION_EVENT_AXIS_LTRIGGER] = "AXIS_LTRIGGER";
	names[AMOTION_EVENT_AXIS_RTRIGGER] = "AXIS_RTRIGGER";
	names[AMOTION_EVENT_AXIS_THROTTLE] = "AXIS_THROTTLE";
	names[AMOTION_EVENT_AXIS_RUDDER] = "AXIS_RUDDER";
	names[AMOTION_EVENT_AXIS_WHEEL] = "AXIS_WHEEL";
	names[AMOTION_EVENT_AXIS_GAS] = "AXIS_GAS";
	names[AMOTION_EVENT_AXIS_BRAKE] = "AXIS_BRAKE";
	names[AMOTION_EVENT_AXIS_DISTANCE] = "AXIS_DISTANCE";
	names[AMOTION_EVENT_AXIS_TILT] = "AXIS_TILT";
	names[AMOTION_EVENT_AXIS_GENERIC_1] = "AXIS_GENERIC_1";
	names[AMOTION_EVENT_AXIS_GENERIC_2] = "AXIS_GENERIC_2";
	names[AMOTION_EVENT_AXIS_GENERIC_3] = "AXIS_GENERIC_3";
	names[AMOTION_EVENT_AXIS_GENERIC_4] = "AXIS_GENERIC_4";
	names[AMOTION_EVENT_AXIS_GENERIC_5] = "AXIS_GENERIC_5";
	names[AMOTION_EVENT_AXIS_GENERIC_6] = "AXIS_GENERIC_6";
	names[AMOTION_EVENT_AXIS_GENERIC_7] = "AXIS_GENERIC_7";
	names[AMOTION_EVENT_AXIS_GENERIC_8] = "AXIS_GENERIC_8";
	names[AMOTION_EVENT_AXIS_GENERIC_9] = "AXIS_GENERIC_9";
	names[AMOTION_EVENT_AXIS_GENERIC_10] = "AXIS_GENERIC_10";
	names[AMOTION_EVENT_AXIS_GENERIC_11] = "AXIS_GENERIC_11";
	names[AMOTION_EVENT_AXIS_GENERIC_12] = "AXIS_GENERIC_12";
	names[AMOTION_EVENT_AXIS_GENERIC_13] = "AXIS_GENERIC_13";
	names[AMOTION_EVENT_AXIS_GENERIC_14] = "AXIS_GENERIC_14";
	names[AMOTION_EVENT_AXIS_GENERIC_15] = "AXIS_GENERIC_15";
	names[AMOTION_EVENT_AXIS_GENERIC_16] = "AXIS_GENERIC_16";

	for (std::map<int, const char*>::iterator iter = names.begin(); iter != names.end(); ++iter)
	{
		float val = AMotionEvent_getAxisValue(motionEvent, iter->first, 0);
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "%s=%f", iter->second, val);
	}
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "onGenericMotionEvent");

		debugMotionEvent(event);

        return 1;
	}
	else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
	{
		int action = AMotionEvent_getAction(event);
		int32_t keyCode = AKeyEvent_getKeyCode(event);
		if (action == AKEY_STATE_UP)
		{
			__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "onKeyUp keyCode=%d", keyCode);
		}
		else if (action == AKEY_STATE_DOWN)
		{
			__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "onKeyDown keyCode=%d", keyCode);
		}

		return 1;
	}
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                        engine->accelerometerSensor, (1000L/60)*1000);
            }
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    // Prepare to monitor accelerometer
    engine.sensorManager = ASensorManager_getInstance();
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
            ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
            state->looper, LOOPER_ID_USER, NULL, NULL);

    if (state->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }

    // loop waiting for stuff to do.

    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER) {
                if (engine.accelerometerSensor != NULL) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
                            &event, 1) > 0) {
                        LOGI("accelerometer: x=%f y=%f z=%f",
                                event.acceleration.x, event.acceleration.y,
                                event.acceleration.z);
                    }
                }
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }

        if (engine.animating) {
            // Done with events; draw next animation frame.
            engine.state.angle += .01f;
            if (engine.state.angle > 1) {
                engine.state.angle = 0;
            }

            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            engine_draw_frame(&engine);
        }
    }
}
//END_INCLUDE(all)
