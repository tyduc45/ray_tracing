#version 460

out vec2 vUV;

// 直接用 gl_VertexID 生成一个覆盖全屏的三角形
const vec2 verts[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

void main()
{
    gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
    vUV = (verts[gl_VertexID] + 1.0) * 0.5;
}
