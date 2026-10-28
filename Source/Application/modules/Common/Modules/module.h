#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include "parameter.h"
#include <vector>
#include "Firmware/Application/Model/ModelParams.h"
#include "Common/Modules/parameter.h"
#include "Common/Modules/parameterbase.h"


namespace Ignite {

	class IParamChangeListener {
	public:
		virtual void onParameterChanged (moduleId_t moduleSrc, ParameterBase* param) = 0;
		virtual ~IParamChangeListener () { }
	};

	class Module {
		struct UndoParam
		{
			paramId_t ParamID;
			AnyType MsgValue;
		};
	public:
		typedef std::vector<ParameterBase*> Parameters_t;
		explicit Module (const char* Name = nullptr, moduleId_t ModuleId = 0, unsigned int NoOfParameters = 0, IParamChangeListener* listener = nullptr)
			: _moduleid (ModuleId)
			, _noOfParameters (NoOfParameters)
			, _listener (listener)
		{
			if (Name) {
#ifdef _WIN64
				strncpy_s (name_, Name, (sizeof (name_) - 1));// strncpy_s making linux build fail, but strncpy gives Windows build warning TBD ?
#else
				strncpy (name_, Name, (sizeof (name_) - 1));
#endif
			}
			if (_noOfParameters) {
				_params.reserve (_noOfParameters);
			}
		}
		virtual ~Module()
		{
			for (auto* parameter : _params) delete parameter;
		}
		Module(const Module&) = delete;
		Module& operator=(const Module&) = delete;
		template <typename T>
		bool AddParameter (paramId_t paramId, T Min, T Max, T Default = (T)0)
		{
			if (_params.size () == _noOfParameters)
				return false;
			if (FindParameter (paramId) != nullptr)
				return false;

			//Name set as module, name not used ; TODO remove  from base class?
			Parameter<T>* param = new Parameter<T> (name_, paramId, *this);
			param->Init (Min, Max, Default);
			_params.push_back (param);
			return true;
		}

		template <typename T>
		bool AddParameter (const char* Name, T Min, T Max, T Default = (T)0)
		{
			if (_params.size () == _noOfParameters)
				return false;
			Parameter<T>* param = new Parameter<T> (Name, _params.size () + 1, *this);
			param->Init (Min, Max, Default);
			_params.push_back (param);
			return true;
		}
		//name not used; TODO remove  from base class ?
		//ParameterBase* FindParameter(const char* Name)
		//{
		//    auto it = std::find_if(_params.begin(), _params.end(), [&](const ParameterBase* param) {
		//        return strcmp(param->GetName(), Name) == 0;
		//    });
		//    if (it != _params.end()) {
		//        return *it;
		//    }
		//    return nullptr;
		//}
		ParameterBase* FindParameter (paramId_t Id)
		{
			auto it = std::find_if (_params.begin (), _params.end (), [&](const ParameterBase* param) {
				return (param->GetParamID () == Id);
				});
			if (it != _params.end ()) {
				return *it;
			}
			return nullptr;
		}
		//name not used ; TODO remove  from base class?
		//paramId_t GetParameterId(const char* Name)
		//{
		//    auto param = FindParameter(Name);
		//    if (param) {
		//        return param->GetParamID();
		//    }
		//    return 0;
		//}
		const char* GetName () const { return name_; }
		unsigned long GetParameterCount () const { return (unsigned long)_params.size (); }
		moduleId_t GetModuleID () const { return _moduleid; }
		void setParam (moduleId_t moduleSrc, paramId_t id, const AnyType& value, bool isPlayMode)
		{
			ParameterBase* parameter = FindParameter (id);
			if (parameter) {
				//   std::cout << "Set Param Module Id: " << (uint16_t)_moduleid << ", Param Id: " << (uint16_t)id << ", Value (" << value.TypeName() << "):" << value.ToString() << std::endl;
				if (parameter->setValue (moduleSrc, value, isPlayMode))
					onParameterChange (moduleSrc, parameter, isPlayMode);

			}
		}
		bool doParamUndo ()
		{
			bool setFromDefault = true;
			paramId_t Id = 0;
			// 1. Get the top param ID which we doing UNDO its value
			// 2. remove the top param
			// 3. Search for the same ID in the list
			// 4. set the older value
			if (!_UndoParamsList.empty ())
			{
				Id = _UndoParamsList.begin ()->ParamID;
				_UndoParamsList.erase (_UndoParamsList.begin ());

				if (!_UndoParamsList.empty ())
				{

					auto it = std::find_if (_UndoParamsList.begin (), _UndoParamsList.end (), [&](const UndoParam param) {
						return (param.ParamID == Id);
						});

					ParameterBase* param = FindParameter (Id);
					if (it != _UndoParamsList.end ())
					{
						param->setValue ((moduleId_t)ModuleName::moduleUndo, it->MsgValue, false);
						onParameterChange ((moduleId_t)ModuleName::moduleUndo, param, false);
						setFromDefault = false;
					}
				}
			}
			if (Id != 0 && setFromDefault == true)
			{
				auto it = std::find_if (_UndoDefaultList.begin (), _UndoDefaultList.end (), [&](const UndoParam param) {
					return (param.ParamID == Id);
					});

				ParameterBase* param = FindParameter (Id);
				if (it != _UndoDefaultList.end ())
				{
					param->setValue ((moduleId_t)ModuleName::moduleUndo, it->MsgValue, false);
					onParameterChange ((moduleId_t)ModuleName::moduleUndo, param, false);
					setFromDefault = false;
				}
			}
			return true;
		}

		std::vector<paramId_t> GetUndoParamsInList ()
		{
			std::vector<paramId_t> retList = {};
			if (!_UndoParamsList.empty ())
			{
				for (auto it = _UndoParamsList.begin (); it < _UndoParamsList.end (); ++it)
					retList.push_back (it->ParamID);


				if (!retList.empty ())
				{
					sort (retList.begin (), retList.end ());
					retList.erase (unique (retList.begin (), retList.end ()), retList.end ());
				}
			}
			return retList;
		}

		void AddToUndoList (ParameterBase* param)
		{
			if (_UndoParamsList.size () >= MAX_UNDO_COUNT)
				_UndoParamsList.pop_back ();

			UndoParam undoparam;
			undoparam.MsgValue = param->getValue ();
			undoparam.ParamID = param->GetParamID ();
			_UndoParamsList.insert (_UndoParamsList.begin (), undoparam);
		}
		void AddToUndoDefaultList (paramId_t Id, AnyType value)
		{
			if (_UndoDefaultList.size () >= MAX_UNDO_COUNT)
				_UndoDefaultList.pop_back ();

			//remove old value
			auto tmp = remove_if (_UndoDefaultList.begin (), _UndoDefaultList.end (), [&](UndoParam param) {return (param.ParamID == Id); });

			UndoParam undoparam;
			undoparam.MsgValue = value;
			undoparam.ParamID = Id;
			_UndoDefaultList.insert (_UndoDefaultList.begin (), undoparam);
		}

		AnyType GetDefaultDataFromList (paramId_t Id)
		{
			auto it = std::find_if (_UndoDefaultList.begin (), _UndoDefaultList.end (), [&](const UndoParam param) {
				return (param.ParamID == Id);
				});

			if (it != _UndoDefaultList.end ())
			{
				return(it->MsgValue);
			}
			return "";
		}

		void ResetToDefaultData()
		{
			if (!_UndoDefaultList.empty())
			{
				for (auto it = _UndoDefaultList.begin(); it < _UndoDefaultList.end(); ++it)
				{
					ParameterBase* param = FindParameter(it->ParamID);
					param->setValue((moduleId_t)ModuleName::moduleUndo, it->MsgValue, false);
					onParameterChange((moduleId_t)ModuleName::moduleUndo, param, false);
				}
			}
		}

		void ClearUndoList ()
		{
			_UndoParamsList.clear ();
		}
		void ClearUndoDefaultList ()
		{
			_UndoDefaultList.clear ();
		}
		virtual void onParameterChange (moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
		{
			_listener->onParameterChanged (moduleSrc, param);
		}
		virtual void setParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode) = 0;
		virtual std::vector<IgniteParamList> getParamList (moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist) = 0;
		virtual void SetCommandParam (moduleId_t moduleSrc, paramId_t param, bool isPlayMode) = 0;
		virtual void getParam (moduleId_t moduleSrc, paramId_t param) = 0;

	private:
		unsigned long _noOfParameters = 0;
		char name_[20] = { '\0' };
		moduleId_t _moduleid = 0;
		Parameters_t _params;
		IParamChangeListener* _listener;
		std::vector<UndoParam> _UndoParamsList;
		std::vector<UndoParam> _UndoDefaultList;
	};
}
