#version 120

attribute vec4 aPosition;
attribute vec4 aNormal;
attribute vec4 aTexCoord0;
attribute vec4 aColour;

uniform mat4 uWorldTransform;
uniform mat4 uClipTransform;
uniform vec4 uColour;

varying vec4 vNormal;
varying vec4 vTexCoord0;
varying vec4 vVisibleMapTexCoord;
varying vec4 vColour;

uniform mat4 uVisibleMapMatrix; 

void main()
{
	vec4 Vertex = ( uWorldTransform * aPosition );
	gl_Position = ( uClipTransform * Vertex  );
	vNormal = aNormal;
	vTexCoord0 = aTexCoord0;
	vVisibleMapTexCoord = uVisibleMapMatrix * Vertex;
	vColour = aColour * uColour;
}
