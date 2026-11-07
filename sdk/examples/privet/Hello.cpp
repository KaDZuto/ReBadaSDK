#include "Hello.h"

using namespace Osp::App;
using namespace Osp::Base;
using namespace Osp::System;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;
using namespace Osp::Graphics;

Hello::Hello()
{
}

Hello::~Hello()
{
}

Application* Hello::CreateInstance(void)
{
	return new Hello();
}

bool Hello::OnAppInitializing(AppRegistry& appRegistry)
{
	Form *pForm = new Form();
	pForm->Construct(FORM_STYLE_NORMAL);
	pForm->SetBackgroundColor(Color::COLOR_WHITE);

	Frame *pFrame = GetAppFrame()->GetFrame();
	pFrame->AddControl(*pForm);

	Rectangle rc = pForm->GetBounds();
	int w = rc.width > 0 ? rc.width : 240;
	int h = rc.height > 0 ? rc.height : 400;

	Label *pLabel = new Label();
	String message("Привет с Линукса");
	pLabel->Construct(Rectangle(10, h / 2 - 60, w - 20, 120), message);
	pLabel->SetBackgroundColor(Color::COLOR_WHITE);
	pLabel->SetTextColor(Color::COLOR_BLACK);
	pLabel->SetTextHorizontalAlignment(ALIGNMENT_CENTER);
	pLabel->SetTextVerticalAlignment(ALIGNMENT_MIDDLE);
	pLabel->SetTextConfig(32, LABEL_TEXT_STYLE_BOLD);
	pForm->AddControl(*pLabel);

	pFrame->SetCurrentForm(*pForm);
	pForm->Draw();
	pForm->Show();

	return true;
}

bool Hello::OnAppTerminating(AppRegistry& appRegistry, bool forcedTermination)
{
	return true;
}

void Hello::OnForeground(void)
{
}

void Hello::OnBackground(void)
{
}

void Hello::OnLowMemory(void)
{
}

void Hello::OnBatteryLevelChanged(BatteryLevel batteryLevel)
{
}

void Hello::OnScreenOn (void)
{
}

void Hello::OnScreenOff (void)
{
}
