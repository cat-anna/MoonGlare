#pragma once

namespace math {
	namespace Constants {
		const float epsilon	= 1.0e-5f;// SIMD_EPSILON;

		namespace pi {
			const float value = (float)(SIMD_PI);// 3.14159f;
			const float half = value / 2.0f;
			const float third = value / 3.0f;
			const float quarter = value / 4.0f;

			const float deg_90 = half;
			const float deg_60 = third;
			const float deg_45 = quarter;
		};
		namespace Earth {
			const float g = 9.8067f;
		};
	};
}
