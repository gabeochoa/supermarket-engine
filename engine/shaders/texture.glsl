

////// ////// ////// ////// ////// ////// ////// //////
//              Texture Shader
////// ////// ////// ////// ////// ////// ////// ////

#type vertex
    #version 400
    in vec3 i_pos;
    in vec4 i_color;
    in vec2 i_texcoord;

    uniform mat4 viewProjection;

    out vec2 v_texcoord;
    out vec4 v_color;

    void main(){
        gl_Position = viewProjection * vec4(i_pos, 1.0);
        v_texcoord = i_texcoord;
        v_color = i_color;
    }

#type fragment
    #version 400
    in vec3 position;
    in vec4 v_color;
    in vec2 v_texcoord;

    uniform float f_tiling;
    uniform vec4 u_color;
    uniform sampler2D u_texture;

    out vec4 frag_color;
    void main(){
        // Debug texcoord
        frag_color = texture(u_texture, v_texcoord) * v_color;
        // frag_color = vec4(v_texcoord, 0.0, 1.0) * v_color;

        // vec4 inter = texture(u_texture, v_texcoord * f_tiling);
        // hide anything with basically no alpha 
        // if(inter.a < 0.01){ discard; }
        // frag_color = inter * u_color;
        // frag_color = inter * u_color;
    }
