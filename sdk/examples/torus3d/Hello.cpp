#include "Hello.h"
#include <math.h>
#include <stdio.h>
#include "font8x12.h"

using namespace Osp::App;
using namespace Osp::Base;
using namespace Osp::Base::Runtime;
using namespace Osp::Graphics;
using namespace Osp::System;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;
using namespace Osp::Graphics::Opengl;

TorusBench::TorusBench()
: __eglDisplay(EGL_DEFAULT_DISPLAY),
  __eglSurface(EGL_NO_SURFACE),
  __eglConfig(null),
  __eglContext(EGL_NO_CONTEXT),
  __pForm(null),
  __pTimer(null),
  __rotX(20.f),
  __rotY(35.f),
  __autoAngle(0.f),
  __dragging(false),
  __lastX(0),
  __lastY(0),
  __triCount(0),
  __lastTick(0),
  __fpsLast(0),
  __frames(0),
  __fps(0.f),
  __frameMs(0.f),
  __baselineMem(0),
  __curMem(0),
  __memLast(0),
  __cpuKeyOk(false),
  __cpuUsage(0),
  __fontTex(0),
  __w(240),
  __h(400)
{
	for (int i = 0; i < VTX * 6; i++) __verts[i] = 0.f;
	for (int i = 0; i < VTX * 6; i++) __idx[i] = 0;
}

TorusBench::~TorusBench()
{
}

Application* TorusBench::CreateInstance(void)
{
	return new TorusBench();
}

bool TorusBench::OnAppInitializing(AppRegistry& appRegistry)
{
	Timer* tempTimer = null;
	result r = E_SUCCESS;

	__pForm = new GlesForm(this);
	if (null == __pForm) { DestroyGL(); return false; }

	r = __pForm->Construct(FORM_STYLE_NORMAL);
	if (IsFailed(r)) { delete __pForm; __pForm = null; return false; }

	r = GetAppFrame()->GetFrame()->AddControl(*__pForm);
	if (IsFailed(r)) { delete __pForm; __pForm = null; return false; }

	__pForm->AddTouchEventListener(*__pForm);
	Touch touch; touch.SetMultipointEnabled(*__pForm, true);

	if (!InitEGL()) { Cleanup(); return false; }
	if (!InitGL()) { Cleanup(); return false; }
	GenTorus();
	BuildFontTexture();

	int m = 0;
	if (E_SUCCESS == RuntimeInfo::GetValue(L"AvailableMemory", m)) {
		__baselineMem = m;
		__curMem = m;
	}

	(void)RuntimeInfo::GetValue(L"AllocatedMemory", __cpuUsage);

	const String keys[] = { L"CpuUsage", L"SystemCpuUsage", L"ProcessCpuUsage" };
	for (int i = 0; i < 3; i++) {
		int v = 0;
		if (E_SUCCESS == RuntimeInfo::GetValue(keys[i], v)) { __cpuKey = keys[i]; __cpuKeyOk = true; break; }
	}

	tempTimer = new Timer;
	if (null == tempTimer) { Cleanup(); return false; }
	r = tempTimer->Construct(*this);
	if (IsFailed(r)) { delete tempTimer; Cleanup(); return false; }
	__pTimer = tempTimer;
	tempTimer = null;

	SystemTime::GetTicks(__lastTick);
	__fpsLast = __lastTick;
	__memLast = __lastTick;

	GetAppFrame()->GetFrame()->SetCurrentForm(*__pForm);
	__pForm->Draw();
	__pForm->Show();
	__pTimer->Start(33);

	return true;
}

bool TorusBench::OnAppTerminating(AppRegistry& appRegistry, bool forcedTermination)
{
	Cleanup();
	return true;
}

void TorusBench::OnForeground(void)
{
	if (__pTimer) __pTimer->Start(33);
}

void TorusBench::OnBackground(void)
{
	if (__pTimer) __pTimer->Cancel();
}

void TorusBench::OnLowMemory(void)
{
}

void TorusBench::OnBatteryLevelChanged(Osp::System::BatteryLevel batteryLevel)
{
}

void TorusBench::OnScreenOn(void)
{
}

void TorusBench::OnScreenOff(void)
{
}

void TorusBench::OnTimerExpired(Timer& timer)
{
	if (!__pTimer) return;
	__pTimer->Start(33);
	if (!Draw()) AppLog("[TorusBench] Draw failed");
}

bool TorusBench::InitEGL()
{
	EGLint numConfigs = 1;
	EGLint cfg[] = { EGL_RED_SIZE,5, EGL_GREEN_SIZE,6, EGL_BLUE_SIZE,5, EGL_ALPHA_SIZE,0,
	                 EGL_DEPTH_SIZE,16, EGL_SURFACE_TYPE,EGL_WINDOW_BIT,
	                 EGL_RENDERABLE_TYPE,EGL_OPENGL_ES_BIT, EGL_NONE };
	EGLint ctx[] = { EGL_CONTEXT_CLIENT_VERSION,1, EGL_NONE };

	eglBindAPI(EGL_OPENGL_ES_API);

	__eglDisplay = eglGetDisplay((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
	if (EGL_NO_DISPLAY == __eglDisplay) { DestroyGL(); return false; }
	if (EGL_FALSE == eglInitialize(__eglDisplay, null, null)) { DestroyGL(); return false; }
	if (EGL_FALSE == eglChooseConfig(__eglDisplay, cfg, &__eglConfig, 1, &numConfigs) || !numConfigs) { DestroyGL(); return false; }

	__eglSurface = eglCreateWindowSurface(__eglDisplay, __eglConfig, (EGLNativeWindowType)__pForm, null);
	if (EGL_NO_SURFACE == __eglSurface) { DestroyGL(); return false; }

	__eglContext = eglCreateContext(__eglDisplay, __eglConfig, EGL_NO_CONTEXT, ctx);
	if (EGL_NO_CONTEXT == __eglContext) { DestroyGL(); return false; }

	if (EGL_FALSE == eglMakeCurrent(__eglDisplay, __eglSurface, __eglSurface, __eglContext)) { DestroyGL(); return false; }
	return true;
}

bool TorusBench::InitGL()
{
	__pForm->GetBounds(__lastX, __lastY, __w, __h); // reuse; reset below
	int x, y;
	__pForm->GetBounds(x, y, __w, __h);

	float aspect = (float)__w / (float)__h;

	glViewport(0, 0, __w, __h);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GLfloat lightPos[] = { 3.f, 4.f, 5.f, 1.f };
	(void)lightPos;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustumf(-aspect, aspect, -1.f, 1.f, 1.5f, 20.f);

	glMatrixMode(GL_MODELVIEW);

	glClearColor(0.04f, 0.06f, 0.12f, 1.f);
	return true;
}

void TorusBench::DestroyGL()
{
	if (__fontTex) { glDeleteTextures(1, &__fontTex); __fontTex = 0; }
	if (EGL_NO_DISPLAY != __eglDisplay) {
		eglMakeCurrent(__eglDisplay, null, null, null);
		if (__eglContext != EGL_NO_CONTEXT) { eglDestroyContext(__eglDisplay, __eglContext); __eglContext = EGL_NO_CONTEXT; }
		if (__eglSurface != EGL_NO_SURFACE) { eglDestroySurface(__eglDisplay, __eglSurface); __eglSurface = EGL_NO_SURFACE; }
		eglTerminate(__eglDisplay);
		__eglDisplay = EGL_NO_DISPLAY;
	}
	__eglConfig = null;
}

void TorusBench::Cleanup()
{
	if (__pTimer) { __pTimer->Cancel(); delete __pTimer; __pTimer = null; }
	DestroyGL();
	if (__pForm) { delete __pForm; __pForm = null; }
}

void TorusBench::TouchDown(int x, int y)
{
	__dragging = true;
	__lastX = x;
	__lastY = y;
}

void TorusBench::TouchMove(int x, int y)
{
	if (!__dragging) return;
	__rotY += (x - __lastX) * 0.7f;
	__rotX += (y - __lastY) * 0.7f;
	__lastX = x;
	__lastY = y;
}

void TorusBench::GenTorus()
{
	const float R = 1.2f, r = 0.45f;
	int idx = 0;
	for (int i = 0; i <= RINGS; i++) {
		float u = (i / (float)RINGS) * 2.f * 3.14159265f;
		for (int j = 0; j <= SIDES; j++) {
			float v = (j / (float)SIDES) * 2.f * 3.14159265f;
			float cu = cosf(u), su = sinf(u);
			float cv = cosf(v), sv = sinf(v);
			__verts[idx*6+0] = (R + r*cv) * cu;
			__verts[idx*6+1] = r * sv;
			__verts[idx*6+2] = (R + r*cv) * su;
			__verts[idx*6+3] = cu * cv;
			__verts[idx*6+4] = sv;
			__verts[idx*6+5] = su * cv;
			idx++;
		}
	}
	int t = 0;
	for (int i = 0; i < RINGS; i++) {
		for (int j = 0; j < SIDES; j++) {
			int a = i*(SIDES+1)+j;
			int b = a + (SIDES+1);
			int c = a + 1;
			int d = b + 1;
			__idx[t++] = a; __idx[t++] = b; __idx[t++] = c;
			__idx[t++] = c; __idx[t++] = b; __idx[t++] = d;
		}
	}
	__triCount = t / 3;
}

void TorusBench::BuildFontTexture()
{
	const int GW = 10, GH = 12;
	const int COLS = 16, ROWS = 6;
	unsigned char buf[COLS*GW * ROWS*GH];
	memset(buf, 0, sizeof(buf));
	for (int code = 32; code < 128; code++) {
		int idx = code - 32;
		int col = idx % COLS;
		int row = idx / COLS;
		for (int y = 0; y < GH; y++) {
			for (int x = 0; x < GW; x++) {
				buf[(row*GH + y)*(COLS*GW) + (col*GW + x)] =
					(unsigned char)(fontGlyphs[idx][y][x] * 255);
			}
		}
	}
	glGenTextures(1, &__fontTex);
	glBindTexture(GL_TEXTURE_2D, __fontTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, COLS*GW, ROWS*GH, 0, GL_ALPHA, GL_UNSIGNED_BYTE, buf);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

bool TorusBench::Draw()
{
	if (__eglDisplay == EGL_NO_DISPLAY) return false;
	eglMakeCurrent(__eglDisplay, __eglSurface, __eglSurface, __eglContext);

	// rotation
	if (!__dragging) __autoAngle += 0.35f;
	long long now;
	SystemTime::GetTicks(now);
	float dt = float(now - __lastTick);
	if (dt <= 0.f) dt = 1.f;
	__lastTick = now;
	__frameMs = dt;
	__frames++;
	if (now - __fpsLast >= 500) {
		__fps = (__frames * 1000.f) / (float)(now - __fpsLast);
		__fpsLast = now;
		__frames = 0;
	}
	if (now - __memLast >= 1000) {
		RuntimeInfo::GetValue(L"AvailableMemory", __curMem);
		if (__cpuKeyOk) RuntimeInfo::GetValue(__cpuKey, __cpuUsage);
		__memLast = now;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawTorus();

	// HUD
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, __fontTex);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	char buf[128];
	DrawText(6, 8, 1.6f, "TORUS 3D BENCH");
	snprintf(buf, sizeof(buf), "FPS %d", (int)__fps); DrawText(6, 24, 1.6f, buf);
	snprintf(buf, sizeof(buf), "MS %d", (int)__frameMs); DrawText(6, 40, 1.6f, buf);
	snprintf(buf, sizeof(buf), "TRI %d", __triCount); DrawText(6, 56, 1.6f, buf);
	if (__baselineMem > 0) {
		int used = __baselineMem - __curMem;
		if (used < 0) used = 0;
		int pct = (used * 100) / __baselineMem;
		snprintf(buf, sizeof(buf), "RAM %d MB %d%%", (int)(__curMem / 1024), pct);
		DrawText(6, 72, 1.6f, buf);
	}
	if (__cpuKeyOk) {
		snprintf(buf, sizeof(buf), "CPU %d%%", __cpuUsage);
		DrawText(6, 88, 1.6f, buf);
	}
	DrawText(6, __h - 28, 1.4f, "DRAG = ROTATE");

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	eglSwapBuffers(__eglDisplay, __eglSurface);
	return true;
}

void TorusBench::DrawTorus()
{
	const float hue = fmodf(__autoAngle, 360.f) / 360.f;
	GLfloat col[] = { 0.3f + 0.7f*hue, 0.6f, 1.0f - 0.7f*hue, 1.f };
	glColor4f(col[0],col[1],col[2],col[3]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.f, 0.f, -5.f);
	glRotatef(__rotX, 1.f, 0.f, 0.f);
	glRotatef(__rotY + __autoAngle, 0.f, 1.f, 0.f);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, 6*sizeof(GLfloat), __verts);
	glNormalPointer(GL_FLOAT, 6*sizeof(GLfloat), __verts + 3);
	glDrawElements(GL_TRIANGLES, __triCount * 3, GL_UNSIGNED_SHORT, __idx);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void TorusBench::DrawText(GLfloat x, GLfloat y, GLfloat size, const char* s)
{
	if (!s || !__fontTex) return;
	const int GW = 10, GH = 12, COLS = 16;
	int len = 0; while (s[len]) len++;
	if (!len) return;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrthof(0.f, (GLfloat)__w, (GLfloat)__h, 0.f, -1.f, 1.f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glBindTexture(GL_TEXTURE_2D, __fontTex);
	glColor4f(1.f, 1.f, 1.f, 1.f);

	static GLfloat vtx[16];
	static GLfloat uv[16];
	// draw one char at a time
	for (int i = 0; s[i]; i++) {
		unsigned char c = (unsigned char)s[i];
		if (c < 32 || c > 127) c = 32;
		int gi = c - 32;
		int col = gi % COLS;
		int row = gi / COLS;
		float u0 = (col * GW) / (float)(COLS * GW);
		float u1 = ((col + 1) * GW) / (float)(COLS * GW);
		float v0 = (row * GH) / (float)(6 * GH);
		float v1 = ((row + 1) * GH) / (float)(6 * GH);

		float cx = x + i * GW * size;
		float cy = y;
		vtx[0]=cx; vtx[1]=cy;
		vtx[2]=cx; vtx[3]=cy+GH*size;
		vtx[4]=cx+GW*size; vtx[5]=cy;
		vtx[6]=cx+GW*size; vtx[7]=cy+GH*size;

		uv[0]=u0; uv[1]=v0;
		uv[2]=u0; uv[3]=v1;
		uv[4]=u1; uv[5]=v0;
		uv[6]=u1; uv[7]=v1;

		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glTexCoordPointer(2, GL_FLOAT, 0, uv);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}
