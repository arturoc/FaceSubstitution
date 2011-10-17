#extension GL_ARB_texture_rectangle : enable
uniform sampler2DRect src, srcBlur, dstBlur;

void main() {
	vec2 pos = gl_TexCoord[0].st;	
	vec4 srcColorBlur = texture2DRect(srcBlur, pos);
	if(srcColorBlur.r > 0.) { // don't copy anything black
		vec3 srcColor = texture2DRect(src, pos).rgb;
		vec4 dstColorBlur = texture2DRect(dstBlur, pos);
		vec3 offset = (dstColorBlur.rgb - srcColorBlur.rgb);
		gl_FragColor = vec4(srcColor + offset, 1.);
	} else {
		gl_FragColor = vec4(0.);
	}
}