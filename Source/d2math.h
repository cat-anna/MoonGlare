/*
 * d2math.h
 *
 *  Created on: 28-11-2013
 *      Author: Paweu
 */

#ifndef D2MATH_H_
#define D2MATH_H_

namespace d2math {

	using vec3 = glm::vec3;
	using vec2 = glm::vec2;

	template <class T>
	inline T sqr(const T &f){
		return f * f;
	}

	template <typename T>
	inline T &min (T &a, T &b){
		if(a < b)return a;
		return b;
	}

	template <typename T>
	inline T &max (T &a, T &b){
		if(a < b)return b;
		return a;
	}

	inline vec3 PointOnCircle_XZ(const vec3 &Center, float Angle, float Radius){
		vec3 out;
		out.x = Center.x + Radius * sin(Angle);
		out.z = Center.z - Radius * cos(Angle);
		return out;
	}

	struct D2Line_XZ{ 
		float A, B, C;
		D2Line_XZ(float a = 0, float b = 0, float c = 0):A(a), B(b), C(c){};
		D2Line_XZ(const vec3 &P1, const vec3 &P2){
			if(P1.z == P2.z){
				A = 0;
				B = 1.0f;
				C = -P2.z;
			} else {
				A = 1.0f;
				B = (P1.x - P2.x);
				if(B != 0) B /= (P1.z - P2.z);
				C = -P1.x - P1.z * B;
			}
		}
	/*
	Ax + By + C = 0;
	By = -Ax - C
	y = -A/B - C/B [B!=0]
	*/
		float AngleWithOX(){
			if(A == 0) return 0;
			if(B == 0) return math::Constants::pi::half;
			return atan(A/B);
		}

		float Distance(const vec3 &point){
			return fabs(A * point.x + B * point.z + C) / sqrt(sqr(A) + sqr(B));
		}

		bool PointOnLine(const vec3 &point, float AllowedError = 0.00001f){
			return fabs(A * point.x + B * point.z + C) < AllowedError;
		}

		D2Line_XZ &NormalThoroughPoint(const vec3 &point, D2Line_XZ &Output){
			Output.A = B;
			Output.B = A;
			Output.C = -Output.A * point.x - Output.B * point.z;
			return Output;
		}

		bool CommonPoint(const D2Line_XZ &line, vec3 &output){
			float W = A * line.B - line.A * B;
			if(W == 0) return false;
			float Wx = line.C * B - line.B * C;
			float Wy = line.A * C - A * line.C;
			output.x = Wx / W;
			output.z = Wy / W;
			return true;
		} 
	};

}

 #endif // D2MATH_H_ 