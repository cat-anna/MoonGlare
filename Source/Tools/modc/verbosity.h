#pragma once

namespace modc {
	template<class ENUM>
	struct ConfStructBase {
		using Enum = ENUM;
		using FlagType = unsigned;

		static void SetConf(Enum flag) { m_Conf |= 1 << static_cast<FlagType>(flag); }
		static void SetAll() { m_Conf = ~FlagType(0); }
		static bool AnyFlag() { return static_cast<bool>(m_Conf); }
	protected:
		static FlagType m_Conf;
	};

	template<class T>
	typename ConfStructBase<T>::FlagType ConfStructBase<T>::m_Conf = 0;

#define VerbosityFlag(NAME) \
	static bool NAME() { const FlagType FLAG = 1 << static_cast<FlagType>(Enum::NAME); return ((m_Conf & (FLAG)) == (FLAG)); } \
	static void Set##NAME() { const FlagType FLAG = 1 << static_cast<FlagType>(Enum::NAME); m_Conf |= FLAG; } \
	static void Unset##NAME() { const FlagType FLAG = 1 << static_cast<FlagType>(Enum::NAME); m_Conf &= ~FLAG; } \

	enum class VerbosityFlags {
		//dummy,
		PrintCurrentStage,

		PrintStep,
		PrintSubStep,
		PrintInfo,

		PrintStageCounter,
	};

	struct Verbosity : public ConfStructBase<VerbosityFlags> {
		VerbosityFlag(PrintCurrentStage);
		VerbosityFlag(PrintStep);
		VerbosityFlag(PrintSubStep);
		VerbosityFlag(PrintInfo);
		VerbosityFlag(PrintStageCounter);
	};

	enum class DumpFlags {
		//dummy,
		Classes,
		Resources,
		FileList,
	};

	struct Dump : public ConfStructBase<DumpFlags> {
		VerbosityFlag(Classes);
		VerbosityFlag(Resources);
		VerbosityFlag(FileList);
	};

	enum class StageFlags {
		//dummy,
		ModuleValidation,
		MediaTest,

		RemoveUnnecessaryFiles,
	};

	struct Stages : public ConfStructBase<StageFlags> {
		VerbosityFlag(ModuleValidation);
		VerbosityFlag(MediaTest);
		VerbosityFlag(RemoveUnnecessaryFiles);
	};

	enum class ScriptsConfFlags {
		DoNotCheck,
		DoNotCompile,
		DoNotStripDebugInfo,
	};

	struct Scripts : public ConfStructBase<ScriptsConfFlags> {
		VerbosityFlag(DoNotCheck);
		VerbosityFlag(DoNotCompile);
		VerbosityFlag(DoNotStripDebugInfo);
	};

#undef VerbosityFlag
} // namespace modc
