#version 130	

uniform float time;
uniform mat4 transform, projection;
uniform sampler2D realTex;

in vec2 pix, uv;

void main()  
{  
    gl_FragColor = vec4(sin(time), cos(time), clamp(sin(time) + cos(time), 0.3, 1.0), 1.0);  
    gl_FragColor = texture(realTex, vec2(uv.x, 1.0 - uv.y));	
}

