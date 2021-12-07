

////// ////// ////// ////// ////// ////// ////// //////
//              Texture Shader
////// ////// ////// ////// ////// ////// ////// ////

#type vertex
    #version 400
    in vec3 i_pos;
    in vec2 i_texcoord;

    uniform mat4 viewProjection;
    uniform mat4 transformMatrix;

    out vec2 v_texcoord;

    void main(){
        gl_Position = viewProjection * transformMatrix * vec4(i_pos, 1.0);
        v_texcoord = i_texcoord;
    }

#type fragment
    #version 400
    in vec3 position;
    in vec2 v_texcoord;

    uniform sampler2D u_texture;

    out vec4 frag_color;
    void main(){
        frag_color = texture(u_texture, v_texcoord);
    }