/*
  * Generated by cppsrc.sh
  * On 2015-03-10 22:23:21,05
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef InputStage_H
#define InputStage_H

namespace modc {
namespace pipeline {

class InputStage : public PipelineStage {
public:
	static PipelineStageInfo GetInfo() {
		return PipelineStageInfo{ "InputStage", &Queue };
	}
protected:	
	static void Queue(Pipeline &p);
};

} // namespace pipeline 
} // namespace modc

#endif
