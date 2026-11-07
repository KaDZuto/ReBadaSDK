#ifndef _TORUSBENCH_H_
#define _TORUSBENCH_H_

#include <FApp.h>
#include <FBase.h>
#include <FGraphics.h>
#include <FGraphicsOpengl.h>
#include <FSystem.h>
#include <FUi.h>

class TorusBench :
	public Osp::App::Application,
	public Osp::System::IScreenEventListener,
	public Osp::Base::Runtime::ITimerEventListener
{
public:
	static Osp::App::Application* CreateInstance(void);

	TorusBench();
	virtual ~TorusBench();

	bool OnAppInitializing(Osp::App::AppRegistry& appRegistry);
	bool OnAppTerminating(Osp::App::AppRegistry& appRegistry, bool forcedTermination = false);
	void OnForeground(void);
	void OnBackground(void);
	void OnLowMemory(void);
	void OnBatteryLevelChanged(Osp::System::BatteryLevel batteryLevel);
	void OnScreenOn(void);
	void OnScreenOff(void);
	void OnTimerExpired(Osp::Base::Runtime::Timer& timer);

	bool Draw();
	void TouchDown(int x, int y);
	void TouchMove(int x, int y);
	void Cleanup();

private:
	bool InitEGL();
	bool InitGL();
	void DestroyGL();
	void GenTorus();
	void DrawTorus();
	void DrawHud();
	void BuildFontTexture();
	void DrawText(Osp::Graphics::Opengl::GLfloat x, Osp::Graphics::Opengl::GLfloat y, Osp::Graphics::Opengl::GLfloat size, const char* s);

	class GlesForm : public Osp::Ui::Controls::Form, public Osp::Ui::ITouchEventListener {
	public:
		GlesForm(TorusBench* app) : __pApp(app) {}
		virtual ~GlesForm(void) {}
		virtual result OnDraw(void) { if (__pApp) __pApp->Draw(); return E_SUCCESS; }
		virtual void OnTouchPressed(const Osp::Ui::Control &source, const Osp::Graphics::Point &currentPosition, const Osp::Ui::TouchEventInfo &touchInfo) { if (__pApp) __pApp->TouchDown(currentPosition.x, currentPosition.y); }
		virtual void OnTouchMoved(const Osp::Ui::Control &source, const Osp::Graphics::Point &currentPosition, const Osp::Ui::TouchEventInfo &touchInfo) { if (__pApp) __pApp->TouchMove(currentPosition.x, currentPosition.y); }
		virtual void OnTouchReleased(const Osp::Ui::Control &source, const Osp::Graphics::Point &currentPosition, const Osp::Ui::TouchEventInfo &touchInfo) {}
		virtual void OnTouchDoublePressed(const Osp::Ui::Control &source, const Osp::Graphics::Point &currentPosition, const Osp::Ui::TouchEventInfo &touchInfo) {}
		virtual void OnTouchFocusIn(const Osp::Ui::Control &source, const Osp::Graphics::Point &currentPosition, const Osp::Ui::TouchEventInfo &touchInfo) {}
		virtual void OnTouchFocusOut(const Osp::Ui::Control &source, const Osp::Graphics::Point &currentPosition, const Osp::Ui::TouchEventInfo &touchInfo) {}
		virtual void OnTouchLongPressed(const Osp::Ui::Control &source, const Osp::Graphics::Point &currentPosition, const Osp::Ui::TouchEventInfo &touchInfo) {}
	private:
		TorusBench* __pApp;
	};

	Osp::Graphics::Opengl::EGLDisplay __eglDisplay;
	Osp::Graphics::Opengl::EGLSurface __eglSurface;
	Osp::Graphics::Opengl::EGLConfig  __eglConfig;
	Osp::Graphics::Opengl::EGLContext __eglContext;
	GlesForm* __pForm;
	Osp::Base::Runtime::Timer* __pTimer;

	float __rotX;
	float __rotY;
	float __autoAngle;
	bool __dragging;
	int __lastX;
	int __lastY;

	static const int RINGS = 24;
	static const int SIDES = 12;
	static const int VTX = (RINGS + 1) * (SIDES + 1);
	Osp::Graphics::Opengl::GLfloat __verts[VTX * 6];
	Osp::Graphics::Opengl::GLushort __idx[VTX * 6];
	int __triCount;

	long long __lastTick;
	long long __fpsLast;
	int __frames;
	float __fps;
	float __frameMs;
	int __baselineMem;
	int __curMem;
	long long __memLast;
	bool __cpuKeyOk;
	Osp::Base::String __cpuKey;
	int __cpuUsage;
	unsigned int __fontTex;
	int __w;
	int __h;
};

#endif
