// OpenGL Mathematics Copyright (c) 2005 - 2011 G-Truc Creation (www.g-truc.net)
// Created : 2005-12-21
// Updated : 2006-11-13
// Licence : This source is under MIT License
// File    : glm/gtx/epsilon.hpp
// Dependency:
// - GLM core
// - GLM_GTX_half

#ifndef glm_gtx_epsilon
#define glm_gtx_epsilon

// Dependency:
#include "../glm.hpp"

#if(defined(GLM_MESSAGES) && !defined(glm_ext))
#       pragma message("GLM: GLM_GTX_epsilon extension included")
#endif

namespace glm {
	namespace gtx {
		namespace epsilon
		{
			template < typename genTypeT, typename genTypeU>
			bool equalEpsilon(
				genTypeT const& x,
				genTypeT const& y,
				genTypeU const& epsilon);

			template < typename genTypeT, typename genTypeU>
			bool notEqualEpsilon(
				genTypeT const& x,
				genTypeT const& y,
				genTypeU const& epsilon);
		}//namespace epsilon
	}//namespace gtx
}//namespace glm

#include "epsilon.inl"

namespace glm { using namespace gtx::epsilon; }

#endif//glm_gtx_epsilon