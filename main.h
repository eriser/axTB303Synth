#ifndef __MINIMAL_PROJECT__
#define __MINIMAL_PROJECT__

#include "axLib.h"
#include "axAudio.h"
#include "axAudioFilter.h"
#include "axAudioBuffer.h"
#include "axAudioBufferPlayer.h"
#include "axAudioWaveTable.h"

class MyAudioSynth: public axAudio
{
public:
    static MyAudioSynth* GetInstance();

    void SetWaveformType(const axAudioWaveTable::axWaveformType& type);
    
    void SetFilterFreq(const double& freq);
    void SetFilterRes(const double& res);
    
    
    void Play();
    
    void SetVolume(const double& volume);
    
    void SetDecay(const double& decay)
    {
        axRange<double> range(44100.0 / 16, 44100.0 / 2);
        _decayTime = range.GetValueFromZeroToOne(decay);
    }
    
    struct Note
    {
        bool slide, up, down, on, accent;
        int note;
    };
    
    const Note* GetNotes() const
    {
        return _notes;
    }
    
    void SetNoteInfo(const int& index, const Note& note)
    {
        _notes[index] = note;
    }
    
    void SetNoteInfoNote(const int& index, const int& note)
    {
        _notes[index].note = note;
    }
    
    void SetNoteInfoOn(const int& index, const bool& on)
    {
        _notes[index].on = on;
    }
    
    void SetNoteInfoUp(const int& index, const bool& up)
    {
        _notes[index].up = up;
    }
    
    void SetNoteInfoDown(const int& index, const bool& down)
    {
        _notes[index].down = down;
    }
    
    void SetTuning(const double& tune)
    {
        _tuning = axClamp<double>(tune, 0.5, 2.0);
    }
    
private:
    MyAudioSynth();
    static MyAudioSynth* _instance;

    axAudioBuffer* _sndBuffer;
    axAudioBufferPlayer* _bufferPlayer;
    axAudioFilter* _filter;
    axAudioWaveTable* _waveTable;

    virtual int CallbackAudio(const float* input,
                              float* output,
                              unsigned long frameCount);
    
    double _bpm;
    int _mesureCount;
    double _mesureTime;
    double _timeCount;
    
    double _decayTime;
    double _decayIndex;
    
    double _volume;
    double _tuning = {1.0};
    Note* _notes;
    
    bool _running = {false};
};


class MyLED : public axPanel
{
public:
    MyLED(axWindow* parent,
          const axRect& rect);
    
    void SetActive(const bool& on);
    void SetOff();
    
    bool IsActive() const
    {
        return (bool)_imgIndex;
    }
    
private:
    void OnPaint();
    int _imgIndex;
    
    
    axImage* _ledImg;
};

class MyNumberPanel : public axPanel
{
public:
    MyNumberPanel(axWindow* parent,
          const axPoint& pos);
    
    void SetNumber(const int& num);
    int GetNumber() const;
    
private:
    void OnPaint();
    int _number;
};

class MyButton : public axButton
{
public:
    MyButton(axWindow* parent,
             const axRect& rect,
             const axButtonEvents& events,
             const axButtonInfo& info,
             const axPoint& led_delta_pos);
    
    void SetLed(MyLED* led);
    
    void SetActive(const bool& on);
    void SetOff();
    
    bool IsActive() const
    {
        return _led->IsActive();
    }
    
    enum MyButtonType
    {
        BUTTON_PALE,
        BUTTON_DARK
    };
    
    axEVENT_ACCESSOR(axButtonMsg, OnClickButton);
    
    class MyButtonBuilder
    {
    public:
        MyButtonBuilder(axWindow* parent,
                        const axSize& size,
                        const axButtonInfo& info,
                        const axPoint& led_delta_pos,
                        const int& x_delta);
        
        MyButton* Create(const axPoint& pos, const axEventFunction& evt);
        MyButton* Create(const axEventFunction& evt);
        
        void SetDelta(const axPoint& delta);
        
        void SetInfo(const axButtonInfo& info)
        {
            _info = info;
        }
        
    private:
        axWindow* _parent;
        axSize _size;
        axButtonInfo  _info;
        axPoint _delta;
        int _xDelta;
        MyButton* _pastBtn;
    };
    
private:
    MyLED* _led;
    bool _active;
    
    void OnClickButton(const axButtonMsg& msg);
};

class MyPreference : public axPanel
{
public:
    MyPreference(const axRect& rect);
    
private:
    // Events.
    virtual void OnPaint();
};

class MyProject: public axPanel
{
public:
    MyProject(axWindow* parent,
              const axRect& rect);

    axEVENT_ACCESSOR(axButtonMsg, OnRunClick);
    axEVENT_ACCESSOR(axButtonMsg, OnNoteClick);
    axEVENT_ACCESSOR(axButtonMsg, OnButtonClick);
    axEVENT_ACCESSOR(axDropMenuMsg, OnWaveChoice);
    
    axEVENT_ACCESSOR(axButtonMsg, OnDownClick);
    axEVENT_ACCESSOR(axButtonMsg, OnUpClick);
    
    axEVENT_ACCESSOR(axKnobMsg, OnTuningChange);
    axEVENT_ACCESSOR(axKnobMsg, OnVolumeChange);
    axEVENT_ACCESSOR(axKnobMsg, OnFreqChange);
    axEVENT_ACCESSOR(axKnobMsg, OnResChange);
    
    axEVENT_ACCESSOR(axKnobMsg, OnDecayChange);
    
    axEVENT_ACCESSOR(axButtonMsg, OnNextEditPattern);
    axEVENT_ACCESSOR(axButtonMsg, OnBackEditPattern);
    
    axEVENT_ACCESSOR(axButtonMsg, OnPreference);
    
    enum MyButtonId
    {
        NOTE_C0,
        NOTE_D,
        NOTE_E,
        NOTE_F,
        NOTE_G,
        NOTE_A,
        NOTE_B,
        NOTE_C1,
        
        DOWN,
        UP,
        ACCENT,
        SLIDE,
        
        NOTE_C0_S,
        NOTE_D_S,
        NOTE_F_S,
        NOTE_G_S,
        NOTE_A_S,
        NUM_OF_BUTTONS
    };
    
private:
    
    void UpdateParameters(const int& index);
    
    // Events.
    virtual void OnPaint();
    
    void OnRunClick(const axButtonMsg& msg);
    void OnNoteClick(const axButtonMsg& msg);
    void OnButtonClick(const axButtonMsg& msg);
    void OnWaveChoice(const axDropMenuMsg& msg);
    
    void OnDownClick(const axButtonMsg& msg);
    void OnUpClick(const axButtonMsg& msg);
    
    void OnVolumeChange(const axKnobMsg& msg);
    void OnTuningChange(const axKnobMsg& msg);
    void OnFreqChange(const axKnobMsg& msg);
    void OnResChange(const axKnobMsg& msg);
    
    void OnDecayChange(const axKnobMsg& msg);
    
    void OnPreference(const axButtonMsg& msg);
    
    
    void OnNextEditPattern(const axButtonMsg& msg);
    void OnBackEditPattern(const axButtonMsg& msg);
    
    axImage* _bgImg;
    MyNumberPanel* _numberPanel;
    MyPreference* _pref;
    std::vector<MyButton*> _btns;
};

#endif // __MINIMAL_PROJECT__
