#include "objects/editext.h"

// EditObject

#ifndef CHOWDREN_USE_EDITOBJ

EditObject::EditObject(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
}

void EditObject::set_text(const std::string & value)
{

}

const std::string & EditObject::get_text()
{
    return empty_string;
}

#else

EditObject::EditObject(int x, int y, int type_id)
: FrameObject(x, y, type_id), edit_flags(0), edit_col(this), font(get_font(14))
{
    collision = &edit_col;
}

void EditObject::update()
{
    if (is_mouse_pressed_once(SDL_BUTTON_LEFT)) {
        int mx, my;
        frame->get_mouse_pos(&mx, &my);
        PointCollision col(mx, my);
        if (collide(&col, &edit_col))
            edit_flags |= FOCUS;
        else
            edit_flags &= ~FOCUS;
    }

    if (!(edit_flags & FOCUS))
        return; 
    text += manager.input;

    if (is_key_pressed_once(SDLK_BACKSPACE))
        text = text.substr(0, text.size() - 1);
    if (is_key_pressed_once(SDLK_RETURN))
        edit_flags &= ~FOCUS;
}

void EditObject::draw()
{
    if (!init_font()) {
        set_visible(false);
        return;
    }

    glDisable(GL_TEXTURE_2D);
    // black background
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // content box
    const float off = 1.0f;
    float x1 = x + off;
    float y1 = y + off;
    float x2 = x + width - off;
    float y2 = y + height - off;

    if (edit_flags & FOCUS)
        glColor4f(0.9f, 0.9f, 0.9f, 1.0f);
    else
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();

    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    float xx = x1 + 5.0f;
    glPushMatrix();
    int yy = y + font->Ascender() + height * 0.5 - font->LineHeight() * 0.5;
    glTranslatef(xx, yy, 0.0f);
    glScalef(1.0f, -1.0f, 1.0f);
    font->Render(text.c_str(), -1, FTPoint(), FTPoint());
    glPopMatrix();
}

void EditObject::set_text(const std::string & value)
{
    text = value;
}

const std::string & EditObject::get_text()
{
    return text;
}

#endif