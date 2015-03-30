#version 430
#extension GL_ARB_compute_variable_group_size : enable

layout(std140, binding=4) buffer pos {
  vec4 positions[];
};

layout(std140, binding=5) buffer vel {
  vec4 velocities[];
};

layout(std140, binding=6) buffer col {
  vec4 colors[];
};

layout(local_size_variable) in;

const vec3 G = vec3(0.0, -9.8, 0.0);
const float DT = 0.1;

uint gid = gl_GlobalInvocationID.x;

void main() {
  vec3 p = positions[gid].xyz;
  vec3 v = velocities[gid].xyz;
  
  vec3 pp = p + v*DT + 0.5*DT*DT*G;
  vec3 vp = v + G*DT;
  
  positions[gid].xyz = pp;
  velocities[gid].xyz = vp;
}