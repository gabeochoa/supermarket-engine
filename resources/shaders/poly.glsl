
////// ////// ////// ////// ////// ////// ////// //////
//              Poly Shader
////// ////// ////// ////// ////// ////// ////// //////

#type vertex
    #version 400
    in vec3 i_pos;
    in vec4 i_color;

    uniform mat4 viewProjection;
    uniform mat4 transformMatrix;

    out vec4 v_color; 

    void main(){
        gl_Position = viewProjection * vec4(i_pos, 1.0);
        v_color = i_color;
    }

#type fragment
    #version 400
    in vec3 position;
    in vec4 v_color;

    out vec4 frag_color;

    void main(){
        frag_color = v_color;
    }
