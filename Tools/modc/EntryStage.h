/*
	Generated by cppsrc.sh
	On 2015-01-04  8:00:39,81
	by Paweu
*/

#pragma once
#ifndef FirstStage_H
#define FirstStage_H

namespace modc {
namespace pipeline {	

class EntryStage : public PipelineStage {
public:
	static PipelineStageInfo GetInfo() {
		return PipelineStageInfo{ "EntryStage", &Queue };
	}
protected:
	static void Queue(Pipeline &p);
};

} // namespace pipeline
} // namespace modc

#endif
