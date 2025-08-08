#version 330

// This is the output that will be sent to the fragment shader.
// The GPU will automatically interpolate this value for each pixel.
out vec2 fragCoord;

// A clever trick to draw a full-screen triangle without any vertex buffers.
// These are the coordinates of a giant triangle in "clip space" (-1 to 1).
const vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

void main() {
    // gl_VertexID is a built-in variable that is 0, 1, or 2 for the 3 vertices.
    // We use it to pick a position from our hardcoded array.
    vec2 pos = positions[gl_VertexID];

    // We calculate the texture coordinates from the clip space position.
    // This will correctly map to the (0,0) to (1,1) range for the screen.
    fragCoord = pos * 0.5 + 0.5;

    // gl_Position is the final output position of the vertex.
    gl_Position = vec4(pos, 0.0, 1.0);
}
