#include "render.h"
#include "glslshader.h"

RenderData render_data;

void Render::init()
{
    render_data.tex1 = false;
    render_data.tex2 = false;
    render_data.tex3 = false;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glEnableVertexAttribArray(POSITION_ATTRIB_IDX);
    // glVertexAttribPointer(POSITION_ATTRIB_IDX, 3, GL_FLOAT, GL_FALSE, 0,
    //                       (void*)&render_data.positions[0]);
    // glEnableVertexAttribArray(COLOR_ATTRIB_IDX);
    // glVertexAttribPointer(COLOR_ATTRIB_IDX, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0,
    //                       (void*)&render_data.colors[0]);

    // glVertexAttribPointer(TEXCOORD1_ATTRIB_IDX, 2, GL_FLOAT, GL_FALSE, 0,
    //                       (void*)&render_data.texcoords[0]);

    // glVertexAttribPointer(TEXCOORD2_ATTRIB_IDX, 2, GL_FLOAT, GL_FALSE, 0,
    //                       (void*)&render_data.texcoords[1]);

    glVertexPointer(2, GL_FLOAT, 0,
                    (void*)&render_data.positions[0]);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, (void*)&render_data.colors[0]);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glClientActiveTexture(GL_TEXTURE0);
    glTexCoordPointer(2, GL_FLOAT, 0, (void*)&render_data.texcoord1[0]); 
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glClientActiveTexture(GL_TEXTURE1);
    glTexCoordPointer(2, GL_FLOAT, 0, (void*)&render_data.texcoord2[0]); 
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glClientActiveTexture(GL_TEXTURE0);
}
