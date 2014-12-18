#include "main.h"
#include <random>

/*******************************************************************************
 * MyAudioSynth.
 ******************************************************************************/
MyAudioSynth* MyAudioSynth::_instance = nullptr;

MyAudioSynth* MyAudioSynth::GetInstance()
{
    return _instance == nullptr ? _instance = new MyAudioSynth() : _instance;
}

MyAudioSynth::MyAudioSynth():
axAudio()
{
    std::string app_path = axApp::GetInstance()->GetAppDirectory();
    std::string snd_path = app_path + ("snare.wav");
    
    _notes = new Note[16];
    
    _sndBuffer = new axAudioBuffer(snd_path);
    _bufferPlayer = new axAudioBufferPlayer(_sndBuffer);
    _waveTable = new axAudioWaveTable();
    _filter = new axAudioFilter();
    _filter->SetFreq(20000.0);
    _filter->SetQ(0.707);
    _filter->SetGain(1.0);
    
    _waveTable->SetWaveformType(axAudioWaveTable::axWAVE_TYPE_SQUARE);
    
    _bpm = 120.0;
    _mesureCount = 0;
    _mesureTime = _bpm / (44100.0 * 60.0);
    _timeCount = 0.0;
    
    axRange<double> range(44100.0 / 16, 44100.0 / 2);
    _decayTime = range.GetValueFromZeroToOne(0.5);
    _decayIndex = 0.0;
    
    _volume = 0.0;
    
    struct Note
    {
        bool slide, up, down, on;
        int note;
    };
    
    for(int i = 0; i < 16; i++)
    {
        _notes[i].on = true;
        _notes[i].slide = false;
        _notes[i].up = false;
        _notes[i].down = false;
        _notes[i].note = 0;
        _notes[i].accent = false;
    }
}

void MyAudioSynth::SetVolume(const double& volume)
{
    _volume = axClamp<double>(volume, 0.0, 1.0);
}

void MyAudioSynth::Play()
{
    _bufferPlayer->Play();
}

void MyAudioSynth::SetWaveformType(const axAudioWaveTable::axWaveformType& type)
{
    _waveTable->SetWaveformType(type);
}

void MyAudioSynth::SetFilterFreq(const double& freq)
{
    _filter->SetFreq(freq);
}

void MyAudioSynth::SetFilterRes(const double& res)
{
    _filter->SetQ(res);
}

int MyAudioSynth::CallbackAudio(const float* input,
                                    float* output,
                                    unsigned long frameCount)
{
    _timeCount += (frameCount);
    
    if(_timeCount >= 44100.0 / 4.0)//_mesureTime)
    {
        _timeCount = 0.0;
        
        double r = _notes[_mesureCount].up ? 2.0 : 1.0;
        double r2 =  _notes[_mesureCount].down ? 0.5 : 1.0;
        _waveTable->SetFreq(r * r2 * _tuning * 110.0 *
                            pow(2.0, _notes[_mesureCount].note / 12.0));
        
        if(_notes[_mesureCount].on)
        {
            
            _decayIndex = 0.0;
        }
        
        ++_mesureCount;
        
        if(_mesureCount >= 16)
        {
            _mesureCount = 0;
        }
        
    }
    
    if(_waveTable != nullptr)
    {
        double v = 0.0;
        
        _waveTable->ProcessBlock(output, frameCount);
        _filter->ProcessStereoBlock(output, frameCount);
        
        for(int i = 0; i < frameCount; i++)
        {
            _decayIndex++;
            
            double decaySample = _decayTime;
            
            if(_decayIndex >= decaySample)
            {
                v = 0.0;
            }
            else
            {
                // Fade in.
                if(_decayIndex <= 30)
                {
                    // Attack.
                    v = axClamp<double>(_decayIndex / 20.0, 0.0, 1.0);
                }
                else
                {
                    // Decay.
                    v = 1.0 - ((_decayIndex) / decaySample);
                    v = axClamp<double>(v, 0.0, 1.0);
                }
                
            }
            
            *output++ = *output * v * _volume;
            *output++ = *output * v * _volume;
        }
    }
    else
    {
        for(int i = 0; i < frameCount; i++)
        {
            *output++ = 0.0f;
            *output++ = 0.0f;
        }
    }

    return 0;
}

/*******************************************************************************
 * MyLED.
 ******************************************************************************/
MyLED::MyLED(axWindow* parent,
             const axRect& rect) :
axPanel(parent, rect)
{
    _ledImg = new axImage("axLED_9x9.png");
    _imgIndex = 0;
}

void MyLED::SetActive(const bool& on)
{
    if(on)
    {
        _imgIndex = 1;
    }
    else
    {
        _imgIndex = 0;
    }
    
    Update();
}

void MyLED::SetOff()
{
    _imgIndex = 0;
    Update();
}

void MyLED::OnPaint()
{
    axGC* gc = GetGC();
    axRect rect = axRect(axPoint(0, 0), GetRect().size);
    

    gc->DrawPartOfImage(_ledImg,
                        axPoint(0, _imgIndex * 9),
                        axSize(9, 9),
                        axPoint(0, 0));
}

/*******************************************************************************
 * MyNumberPanel.
 ******************************************************************************/
MyNumberPanel::MyNumberPanel(axWindow* parent,
                             const axPoint& pos) :
axPanel(parent, axRect(pos, axSize(28, 15))),
_number(1)
{

}

void MyNumberPanel::SetNumber(const int& num)
{
    _number = axClamp<int>(num, 0, 99);
    Update();
}

int MyNumberPanel::GetNumber() const
{
    return _number;
}

void MyNumberPanel::OnPaint()
{
    axGC* gc = GetGC();
    axRect rect = axRect(axPoint(0, 0), GetRect().size);
    
    gc->SetColor(axColor(0.3, 0.0, 0.0, 1.0));
    gc->DrawRectangle(rect);
    
    gc->SetColor(axColor(0.4, 0.0, 0.0, 1.0));
    gc->SetFontType(std::string("digital-7 (mono).ttf"));
    gc->SetFontSize(16);
    gc->DrawChar('0', axPoint(5, -4));
    gc->DrawChar('0', axPoint(13, -4));
    
    
    gc->SetColor(axColor(0.95, 0.0, 0.0, 1.0));
    
    if(_number > 9)
    {
        gc->DrawChar(std::to_string(_number)[0], axPoint(5, -4));

        gc->DrawChar(std::to_string(_number)[1], axPoint(13, -4));
    }
    else
    {
        gc->DrawChar(std::to_string(_number)[0], axPoint(13, -4));
    }
    
    gc->SetColor(axColor(0.4, 0.0, 0.0, 1.0));
    gc->DrawRectangleContour(rect.GetInteriorRect(axPoint(1, 1)));
    
    gc->SetColor(axColor(0.0, 0.0, 0.0, 1.0));
    gc->DrawRectangleContour(rect);
    
}

/*******************************************************************************
 * MyButton.
 ******************************************************************************/
MyButton::MyButton(axWindow* parent,
                   const axRect& rect,
                   const axButtonEvents& events,
                   const axButtonInfo& info,
                   const axPoint& led_delta_pos):
axButton(parent, rect, axButtonEvents(GetOnClickButton()), info),
_led(nullptr),
_active(false)
{
    if(events.button_click)
    {
        AddConnection(2, events.button_click);
    }
    
    
    _led = new MyLED(parent, rect.position + led_delta_pos);
}

void MyButton::SetLed(MyLED* led)
{
    _led = led;
}

void MyButton::SetActive(const bool& on)
{
    _led->SetActive(on);
}

void MyButton::SetOff()
{
    _led->SetActive(false);
}

void MyButton::OnClickButton(const axButtonMsg& msg)
{
    if(_active)
    {
        _led->SetActive(false);
    }
    else
    {
       _led->SetActive(true);
    }
    
    _active = !_active;
    
    PushEvent(2, new axButtonMsg(msg));
}

MyButton::MyButtonBuilder::MyButtonBuilder(axWindow* parent,
                                           const axSize& size,
                                           const axButtonInfo& info,
                                           const axPoint& led_delta_pos,
                                           const int& delta):
_parent(parent),
_size(size),
_info(info),
_delta(led_delta_pos),
_xDelta(delta),
_pastBtn(nullptr)
{
    
}

void MyButton::MyButtonBuilder::SetDelta(const axPoint& delta)
{
    _delta = delta;
}

MyButton* MyButton::MyButtonBuilder::Create(const axPoint& pos,
                                            const axEventFunction& evt)
{
    
    return _pastBtn = new MyButton(_parent, axRect(pos, _size), axButtonEvents(evt),
                        _info, _delta);
}

MyButton* MyButton::MyButtonBuilder::Create(const axEventFunction& evt)
{
    return _pastBtn = new MyButton(_parent,
                                   axRect(_pastBtn->GetNextPosRight(_xDelta),
                                          _size), axButtonEvents(evt),
                        _info, _delta);
}

/*******************************************************************************
 * MyPreference.
 ******************************************************************************/
MyPreference::MyPreference(const axRect& rect) :
axPanel(3, nullptr, rect)
{

}

void MyPreference::OnPaint()
{
    axGC* gc = GetGC();
    axRect rect = axRect(axPoint(0, 0), GetRect().size);
    
    
    gc->SetColor(axColor("#9B9A9A"));
    gc->DrawRectangle(rect);
    
    gc->SetColor(axColor(0.0, 0.0, 0.0));
    gc->DrawString(std::string("Audio"), axPoint(20, 20));
    
    gc->SetColor(axColor(0.0, 0.0, 0.0));
    gc->DrawRectangleContour(rect);
}

/*******************************************************************************
 * MyProject.
 ******************************************************************************/
MyProject::MyProject(axWindow* parent, const axRect& rect):
axPanel(parent, rect)
{
    std::string app_path = axApp::GetInstance()->GetAppDirectory();
    
    _bgImg = new axImage(app_path + std::string("tb303.png"));
    
    axButtonInfo btn_info(axColor(0.4, 0.4, 0.4, 1.0),
                          axColor(0.5, 0.5, 0.5, 1.0),
                          axColor(0.3, 0.3, 0.3, 1.0),
                          axColor(0.4, 0.4, 0.4, 1.0),
                          axColor(0.0, 0.0, 0.0, 1.0),
                          axColor(0.0, 0.0, 0.0, 1.0));
    
    axButtonInfo btn_info_dark(axColor(0.2, 0.2, 0.2, 1.0),
                               axColor(0.3, 0.3, 0.3, 1.0),
                               axColor(0.1, 0.1, 0.1, 1.0),
                               axColor(0.2, 0.2, 0.2, 1.0),
                               axColor(0.0, 0.0, 0.0, 1.0),
                               axColor(0.0, 0.0, 0.0, 1.0));

    MyButton::MyButtonBuilder btnBuilder(this, axSize(18, 36),
                                         btn_info, axPoint(4, -15), 28);
    
    _btns.resize(NUM_OF_BUTTONS);
    
    _btns[NOTE_C0] = btnBuilder.Create(axPoint(206, 167), GetOnNoteClick());
    _btns[NOTE_C0]->SetActive(true);
    _btns[NOTE_D] = btnBuilder.Create(GetOnNoteClick());
    _btns[NOTE_E] = btnBuilder.Create(GetOnNoteClick());
    _btns[NOTE_F] = btnBuilder.Create(GetOnNoteClick());
    _btns[NOTE_G] = btnBuilder.Create(GetOnNoteClick());
    _btns[NOTE_A] = btnBuilder.Create(GetOnNoteClick());
    _btns[NOTE_B] = btnBuilder.Create(GetOnNoteClick());
    _btns[NOTE_C1] = btnBuilder.Create(GetOnNoteClick());
    
    _btns[DOWN] = btnBuilder.Create(GetOnDownClick());
    _btns[UP] = btnBuilder.Create(GetOnUpClick());
    _btns[ACCENT] = btnBuilder.Create(GetOnButtonClick());
    _btns[SLIDE] = btnBuilder.Create(GetOnButtonClick());
    
    // Dark buttons.
    btnBuilder.SetInfo(btn_info_dark);
    
    _btns[NOTE_C0_S] = btnBuilder.Create(axPoint(230, 113), GetOnNoteClick());
    _btns[NOTE_D_S] = btnBuilder.Create(GetOnNoteClick());
    _btns[NOTE_F_S] = btnBuilder.Create(axPoint(368, 113), GetOnNoteClick());
    _btns[NOTE_G_S] = btnBuilder.Create(GetOnNoteClick());
    _btns[NOTE_A_S] = btnBuilder.Create(GetOnNoteClick());
    
    axSize horiBtnSize(36, 18);
    
    axButton* wave = new axButton(this,
                                  axRect(axPoint(154, 25), axSize(32, 15)),
                                  axEventFunction(),
                                  btn_info);
    
    MyButton* pitch = new MyButton(this,
                                   axRect(axPoint(132, 120), horiBtnSize),
                                   axEventFunction(),
                                   btn_info_dark,
                                   axPoint(15, -17));
    
    axButton* clear = new axButton(this,
                                   axRect(axPoint(50, 120), horiBtnSize),
                                   axEventFunction(),
                                   btn_info);
    
    MyButton* run = new MyButton(this,
                                 axRect(axPoint(46, 169), axSize(42, 35)),
                                 axEventFunction(GetOnRunClick()),
                                 btn_info,
                                 axPoint(15, -17));
    
    axButton* back = new axButton(this,
                                  axRect(axPoint(773, 120), horiBtnSize),
                                  axButtonEvents(GetOnBackEditPattern()),
                                  btn_info);
    
    axButton* slide = new axButton(this,
                                   axRect(axPoint(707, 111), axSize(32, 15)),
                                   axEventFunction(),
                                   btn_info);
    
    axButton* next = new axButton(this,
                                  axRect(axPoint(773, 187), horiBtnSize),
                                  axButtonEvents(GetOnNextEditPattern()),
                                  btn_info);

    axKnobInfo knob_info(axColor(0.3, 0.3, 0.3, 0.0),
                         axColor(0.3, 0.3, 0.3, 0.0),
                         axColor(0.3, 0.3, 0.3, 0.0),
                         128,
                         axSize(46, 46),
                         app_path + std::string("knob_dark.png"),
                         app_path + std::string("knob_dark.png"));
    
    
    knob_info.knob_size = axSize(50, 50);
    knob_info.img_path = app_path + std::string("axKnobTB303_50x50.png");
    knob_info.selected_img_path = app_path + std::string("axKnobTB303_50x50.png");
    
    axSize knob_size(50, 50);
    
    axKnob* speed = new axKnob(this, axRect(axPoint(15, 25), knob_size),
                               axKnobEvents(),
                               knob_info);
    speed->SetValue(0.5);
    
    axKnob* volume = new axKnob(this, axRect(speed->GetNextPosRight(10), knob_size),
                                axKnobEvents(GetOnVolumeChange()),
                                knob_info);
    volume->SetValue(0.0);
    
    
    axKnob* f1 = new axKnob(this, axRect(axPoint(239, 25), knob_size),
                           axKnobEvents(GetOnTuningChange()),
                           knob_info);
    f1->SetValue(0.5);
    
    axKnob* freq = new axKnob(this, axRect(f1->GetNextPosRight(10), knob_size),
                           axKnobEvents(GetOnFreqChange()),
                           knob_info);
    freq->SetValue(1.0);
    
    axKnob* res = new axKnob(this, axRect(freq->GetNextPosRight(10), knob_size),
                           axKnobEvents(GetOnResChange()),
                           knob_info);
    res->SetValue(0.0);
    
    axKnob* f4 = new axKnob(this, axRect(res->GetNextPosRight(10), knob_size),
                           axKnobEvents(),
                           knob_info);
    f4->SetValue(0.5);
    
    axKnob* f5 = new axKnob(this, axRect(f4->GetNextPosRight(10), knob_size),
                            axKnobEvents(GetOnDecayChange()),
                            knob_info);
    f5->SetValue(0.5);
    
    axKnob* f6 = new axKnob(this, axRect(f5->GetNextPosRight(10), knob_size),
                            axKnobEvents(),
                            knob_info);
    f6->SetValue(0.5);
    
    _numberPanel = new MyNumberPanel(this, axPoint(778, 165));

    
    axButton* prefBtn = new axButton(this,
                                     axRect(axPoint(820, 10),
                                            axSize(20, 20)),
                                     axButtonEvents(GetOnPreference()),
                                     axBUTTON_TRANSPARENT,
                                     "settings.png",
                                     "",
                                     axBUTTON_SINGLE_IMG);

    
    _pref = new MyPreference(axRect(640, 10, 170, 58));
    _pref->Hide();
}

void MyProject::OnVolumeChange(const axKnobMsg& msg)
{
    MyAudioSynth::GetInstance()->SetVolume(msg.GetValue());
}

void MyProject::OnTuningChange(const axKnobMsg& msg)
{
    axRange<double> range(0.5, 2.0);
    MyAudioSynth::GetInstance()->SetTuning(range.GetValueFromZeroToOne(msg.GetValue()));
}

void MyProject::OnWaveChoice(const axDropMenuMsg& msg)
{
    MyAudioSynth* audio = MyAudioSynth::GetInstance();
    std::string m = msg.GetMsg();
    if(m == "Sine")
    {
        audio->SetWaveformType(axAudioWaveTable::
                               axWaveformType::axWAVE_TYPE_SINE);
    }
    else if(m == "Triangle")
    {
        audio->SetWaveformType(axAudioWaveTable::
                               axWaveformType::axWAVE_TYPE_TRIANGLE);
    }
    else if(m == "Square")
    {
        audio->SetWaveformType(axAudioWaveTable::
                               axWaveformType::axWAVE_TYPE_SQUARE);
    }
    else if(m == "Saw")
    {
        audio->SetWaveformType(axAudioWaveTable::
                               axWaveformType::axWAVE_TYPE_SAW);
    }
}

void MyProject::OnUpClick(const axButtonMsg& msg)
{
    MyButton* sender = static_cast<MyButton*>(msg.GetSender());
//    sender->SetActive(true);
    
    MyAudioSynth::GetInstance()->SetNoteInfoUp(_numberPanel->GetNumber() - 1,
                                                sender->IsActive());
}

void MyProject::OnDownClick(const axButtonMsg& msg)
{
    MyButton* sender = static_cast<MyButton*>(msg.GetSender());
//    sender->SetActive(true);
    
    MyAudioSynth::GetInstance()->SetNoteInfoDown(_numberPanel->GetNumber() - 1,
                                               sender->IsActive());
}

void MyProject::OnNoteClick(const axButtonMsg& msg)
{
//    std::cout << "Note click : " << msg.GetSender()->GetId() << std::endl;
    
    std::vector<MyButtonId> ids = { NOTE_C0, NOTE_D, NOTE_E, NOTE_F, NOTE_G,
                                    NOTE_A, NOTE_B, NOTE_C1, NOTE_C0_S,
                                    NOTE_D_S, NOTE_F_S, NOTE_G_S, NOTE_A_S };
    
    MyButtonId noteId;
    
    for(auto& n : ids)
    {
        MyButton* sender = static_cast<MyButton*>(msg.GetSender());
        if(_btns[n] != sender)
        {
            _btns[n]->SetActive(false);
        }
        else
        {
            noteId = n;
            sender->SetActive(true);
        }
    }
    
    int index = 0;
    
    switch(noteId)
    {
        case NOTE_C0: index = 0; break;
        case NOTE_C0_S: index = 1; break;
        case NOTE_D: index = 2; break;
        case NOTE_D_S: index = 3; break;
        case NOTE_E: index = 4; break;
        case NOTE_F: index = 5; break;
        case NOTE_F_S: index = 6; break;
        case NOTE_G: index = 7; break;
        case NOTE_G_S: index = 8; break;
        case NOTE_A: index = 9; break;
        case NOTE_A_S: index = 10; break;
        case NOTE_B: index = 11; break;
        case NOTE_C1: index = 12; break;
    }
    
    
    std::cout << "NOTE CLICK : " << _numberPanel->GetNumber() - 1 << " " << index << std::endl;
    MyAudioSynth::GetInstance()->SetNoteInfoNote(_numberPanel->GetNumber() - 1,
                                                 index);
}

void MyProject::OnRunClick(const axButtonMsg& msg)
{
    if(static_cast<MyButton*>(msg.GetSender())->IsActive())
    {
        MyAudioSynth::GetInstance()->StartAudio();
    }
    else
    {
        MyAudioSynth::GetInstance()->StopAudio();
    }
}

void MyProject::OnButtonClick(const axButtonMsg& msg)
{
    std::cout << "Button click." << std::endl;
}

void MyProject::OnDecayChange(const axKnobMsg& msg)
{
    MyAudioSynth::GetInstance()->SetDecay(msg.GetValue());
}

void MyProject::OnFreqChange(const axKnobMsg& msg)
{
    axRange<double> freqRange(100.0, 20000.0);
    double f = freqRange.GetValueFromZeroToOne(msg.GetValue());
    
    MyAudioSynth::GetInstance()->SetFilterFreq(f);
}

void MyProject::OnResChange(const axKnobMsg& msg)
{
    axRange<double> qRange(0.707, 10.0);
    double r = qRange.GetValueFromZeroToOne(msg.GetValue());
    
    MyAudioSynth::GetInstance()->SetFilterRes(r);
}

void MyProject::UpdateParameters(const int& index)
{
    std::vector<MyButtonId> ids = { NOTE_C0, NOTE_D, NOTE_E, NOTE_F, NOTE_G,
        NOTE_A, NOTE_B, NOTE_C1, NOTE_C0_S,
        NOTE_D_S, NOTE_F_S, NOTE_G_S, NOTE_A_S };
    
    for(auto& n : ids)
    {
        _btns[n]->SetActive(false);
    }
    
    const MyAudioSynth::Note* notes = MyAudioSynth::GetInstance()->GetNotes();
    
    switch(notes[index].note)
    {
        case 0: _btns[NOTE_C0]->SetActive(true); break;
        case 1: _btns[NOTE_C0_S]->SetActive(true); break;
        case 2: _btns[NOTE_D]->SetActive(true); break;
        case 3: _btns[NOTE_D_S]->SetActive(true); break;
        case 4: _btns[NOTE_E]->SetActive(true); break;
        case 5: _btns[NOTE_F]->SetActive(true); break;
        case 6: _btns[NOTE_F_S]->SetActive(true); break;
        case 7: _btns[NOTE_G]->SetActive(true); break;
        case 8: _btns[NOTE_G_S]->SetActive(true); break;
        case 9: _btns[NOTE_A]->SetActive(true); break;
        case 10: _btns[NOTE_A_S]->SetActive(true); break;
        case 11: _btns[NOTE_B]->SetActive(true); break;
        case 12: _btns[NOTE_C1]->SetActive(true); break;
    }
    
    _btns[DOWN]->SetActive(notes[index].down);
    _btns[UP]->SetActive(notes[index].up);
    _btns[ACCENT]->SetActive(notes[index].accent);
    _btns[SLIDE]->SetActive(notes[index].slide);
    
    Update();
}

void MyProject::OnNextEditPattern(const axButtonMsg& msg)
{
    int num = _numberPanel->GetNumber();
    ++num;
    if(num > 16)
    {
        num = 1;
    }
    
    _numberPanel->SetNumber(num);
    
    UpdateParameters(num - 1);
}

void MyProject::OnBackEditPattern(const axButtonMsg& msg)
{
    int num = _numberPanel->GetNumber();
    --num;
    if(num < 1)
    {
        num = 16;
    }
    
    _numberPanel->SetNumber(num);
    
    UpdateParameters(num - 1);
}

void MyProject::OnPreference(const axButtonMsg& msg)
{
    if(_pref->IsShown())
    {
        _pref->Hide();
    }
    else
    {
        _pref->Show();
    }
}

void MyProject::OnPaint()
{
    axGC* gc = GetGC();
    axRect rect0(axPoint(0, 0), GetRect().size);
    
    gc->SetColor(axColor(0.4, 0.4, 0.4), 1.0);
    gc->DrawRectangle(rect0);
    
    gc->DrawImage(_bgImg, axPoint(0, 0));

    gc->SetColor(axColor(0.0, 0.0, 0.0), 1.0);
    gc->DrawRectangleContour(rect0);

}

void axMain::MainEntryPoint(axApp* app)
{
    MyAudioSynth* audio = MyAudioSynth::GetInstance();

    MyProject* myProject = new MyProject(nullptr, axRect(0, 0, 856, 273));
    
    audio->InitAudio();
//    audio->StartAudio();
}


