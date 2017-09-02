#include "c_api.h"
#include "qt_api_gen.h"
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QPainter>

struct PrivData {
    QWidget* parent;
    void* user_data;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void widget_show(void* self_c) { 
    QWidget* qt_data = (QWidget*)self_c;
    qt_data->show();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_button_show(void* self_c) { 
    QPushButton* qt_data = (QPushButton*)self_c;
    qt_data->show();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void connect_push_button_released(void* object, void* user_data, void (*callback)(void* self_c)) {
    QSlotWrapperSignal_self_void* wrap = new QSlotWrapperSignal_self_void(user_data, (Signal_self_void)callback);
    QObject* q_obj = (QObject*)object;
    QObject::connect(q_obj, SIGNAL(released()), wrap, SLOT(method()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_button_set_text(void* self_c, const char* text) { 
    QPushButton* qt_data = (QPushButton*)self_c;
    qt_data->setText(QString::fromLatin1(text));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void push_button_set_flat(void* self_c, bool flat) { 
    QPushButton* qt_data = (QPushButton*)self_c;
    qt_data->setFlat(flat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void connect_slider_value_changed(void* object, void* user_data, void (*callback)(void* self_c, int value)) {
    QSlotWrapperSignal_self_i32_void* wrap = new QSlotWrapperSignal_self_i32_void(user_data, (Signal_self_i32_void)callback);
    QObject* q_obj = (QObject*)object;
    QObject::connect(q_obj, SIGNAL(valueChanged(int)), wrap, SLOT(method(int)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void painter_draw_line(void* self_c, int x1, int y1, int x2, int y2) { 
    QPainter* qt_data = (QPainter*)self_c;
    qt_data->drawLine(x1, y1, x2, y2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct PUWidget s_widget = {
    widget_show,
    0,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct PUPushButton s_push_button = {
    push_button_show,
    connect_push_button_released,
    push_button_set_text,
    push_button_set_flat,
    0,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct PUSlider s_slider = {
    connect_slider_value_changed,
    0,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct PUPainter s_painter = {
    painter_draw_line,
    0,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, typename QT> T* create_func(T* struct_data, void* priv_data) {
    PrivData* data = (PrivData*)priv_data;
    QT* qt_obj = new QT(data->parent);
    T* ctl = new T;
    memcpy(ctl, struct_data, sizeof(T));
    ctl->priv_data = qt_obj;
    return ctl;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct PUWidget* create_widget(void* priv_data) {
    return create_func<struct PUWidget, QWidget>(&s_widget, priv_data);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct PUPushButton* create_push_button(void* priv_data) {
    return create_func<struct PUPushButton, QPushButton>(&s_push_button, priv_data);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct PUSlider* create_slider(void* priv_data) {
    return create_func<struct PUSlider, QSlider>(&s_slider, priv_data);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct PUPainter* create_painter(void* priv_data) {
    return create_func<struct PUPainter, QPainter>(&s_painter, priv_data);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static struct PU s_pu = {
    create_widget,
    create_push_button,
    create_slider,
    create_painter,
};


struct PU* PU_create_instance(void* user_data, QWidget* parent) {
    struct PU* instance = new PU;
    memcpy(instance, &s_pu, sizeof(PU));
    PrivData* priv_data = new PrivData;
    priv_data->parent = parent;
    priv_data->user_data = user_data;
    instance->priv_data = priv_data;
    return instance;
}

