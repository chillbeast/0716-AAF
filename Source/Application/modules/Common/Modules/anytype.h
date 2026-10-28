#pragma once

#include <sstream>
#include <string>
#include <variant>
#include <type_traits>
#include <cstdint>
#include "common_defs/source/IgniteMessage.h"

namespace Ignite {
	class model {
	public:
		static MTDataType GetType (const bool& value)
		{
			return MTDataType::BOOL;
		}
		static MTDataType GetType (const int8_t& value)
		{
			return MTDataType::INT8;
		}

		static MTDataType GetType (const uint8_t& value)
		{
			return MTDataType::UINT8;
		}

		static MTDataType GetType (const int16_t& value)
		{
			return MTDataType::INT16;
		}

		static MTDataType GetType (const uint16_t& value)
		{
			return MTDataType::UINT16;
		}

		static MTDataType GetType (const int32_t& value)
		{
			return MTDataType::INT32;
		}

		static MTDataType GetType (const uint32_t& value)
		{
			return MTDataType::UINT32;
		}

		static MTDataType GetType (const int64_t& value)
		{
			return MTDataType::INT64;
		}

		static MTDataType GetType (const uint64_t& value)
		{
			return MTDataType::UINT64;
		}

		static MTDataType GetType (const float& value)
		{
			return MTDataType::FLOAT32;
		}

		static MTDataType GetType (const std::string& value)
		{
			return MTDataType::STRING;
		}

		static const char* GetTypeName (MTDataType type)
		{
			static const char* typenames[] = { "bool", "i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "float", "str" };
			return typenames[(uint8_t)type];
		}

		static std::string ToString (uint8_t value)
		{
			std::stringstream ss;
			ss << (int)(value);
			return ss.str ();
		}

		static std::string ToString (int8_t value)
		{
			std::stringstream ss;
			ss << (int)(value);
			return ss.str ();
		}

		template <typename T>
		static std::string ToString (T value)
		{
			std::stringstream ss;
			ss << value;
			return ss.str ();
		}
	};

	class AnyTypeBase {
	public:
		virtual ~AnyTypeBase () { }
		virtual bool operator<(const AnyTypeBase& rhs) = 0;
		virtual bool operator<=(const AnyTypeBase& rhs) = 0;
		virtual bool operator>(const AnyTypeBase& rhs) = 0;
		virtual bool operator>=(const AnyTypeBase& rhs) = 0;
		virtual bool operator==(const AnyTypeBase& rhs) = 0;
		virtual bool operator!=(const AnyTypeBase& rhs) = 0;
		virtual MTDataType GetType () const = 0;
		virtual std::string ToString () = 0;
		virtual const char* TypeName () const = 0;
	};

	template <typename T>
	class AnyTypeStore : public AnyTypeBase {
	public:
		AnyTypeStore (T value)
			: _data (value)
		{
		}

		AnyTypeStore () = default;
		AnyTypeBase& operator=(const AnyTypeBase& rhs)
		{
			if (this != &rhs) {
				if (rhs.GetType () == model::GetType (_data)) {
					const AnyTypeStore<T>& rhsvalue = (const AnyTypeStore<T>&)rhs;
					_data = rhsvalue._data;
				}
			}
			return *this;
		}

		bool operator<(const AnyTypeBase& rhs) override
		{
			if (rhs.GetType () == model::GetType (_data)) {
				const AnyTypeStore<T>& rhsvalue = (const AnyTypeStore<T>&)rhs;
				return _data < rhsvalue._data;
			}
			return false;
		}

		bool operator<=(const AnyTypeBase& rhs) override
		{
			if (rhs.GetType () == model::GetType (_data)) {
				const AnyTypeStore<T>& rhsvalue = (const AnyTypeStore<T>&)rhs;
				return _data <= rhsvalue._data;
			}
			return false;
		}

		bool operator>(const AnyTypeBase& rhs) override
		{
			if (rhs.GetType () == model::GetType (_data)) {
				const AnyTypeStore<T>& rhsvalue = (const AnyTypeStore<T>&)rhs;
				return _data > rhsvalue._data;
			}
			return false;
		}

		bool operator>=(const AnyTypeBase& rhs) override
		{
			if (rhs.GetType () == model::GetType (_data)) {
				const AnyTypeStore<T>& rhsvalue = (const AnyTypeStore<T>&)rhs;
				return _data >= rhsvalue._data;
			}
			return false;
		}

		bool operator==(const AnyTypeBase& rhs) override
		{
			if (rhs.GetType () == model::GetType (_data)) {
				const AnyTypeStore<T>& rhsvalue = (const AnyTypeStore<T>&)rhs;
				return _data == rhsvalue._data;
			}
			return false;
		}

		bool operator!=(const AnyTypeBase& rhs) override
		{
			if (rhs.GetType () == model::GetType (_data)) {
				const AnyTypeStore<T>& rhsvalue = (const AnyTypeStore<T>&)rhs;
				return _data != rhsvalue._data;
			}
			return false;
		}

		MTDataType GetType () const override
		{
			return model::GetType (_data);
		}

		std::string ToString () override
		{
			switch (GetType ()) {
				case MTDataType::INT8:
					return model::ToString (_data);
				case MTDataType::UINT8:
					return model::ToString (_data);
					break;
			}

			return model::ToString<T> (_data);
		}

		const char* TypeName () const override
		{
			return model::GetTypeName (GetType ());
		}
		T _data;
	};

	// Own each message value and preserve its type across queue copies and undo.
	class AnyType {
	public:
		AnyType() : value(uint64_t(0)) {}
		explicit AnyType(MTDataType type) : AnyType()
		{
			switch (type) {
			case MTDataType::BOOL: value = false; break;
			case MTDataType::INT8: value = int8_t(0); break;
			case MTDataType::UINT8: value = uint8_t(0); break;
			case MTDataType::INT16: value = int16_t(0); break;
			case MTDataType::UINT16: value = uint16_t(0); break;
			case MTDataType::INT32: value = int32_t(0); break;
			case MTDataType::UINT32: value = uint32_t(0); break;
			case MTDataType::INT64: value = int64_t(0); break;
			case MTDataType::UINT64: value = uint64_t(0); break;
			case MTDataType::FLOAT32: value = 0.0f; break;
			case MTDataType::STRING: value = std::string(); break;
			}
		}
		AnyType(bool v) : value(v) {}
		AnyType(int8_t v) : value(v) {}
		AnyType(uint8_t v) : value(v) {}
		AnyType(int16_t v) : value(v) {}
		AnyType(uint16_t v) : value(v) {}
		AnyType(int32_t v) : value(v) {}
		AnyType(uint32_t v) : value(v) {}
		AnyType(int64_t v) : value(v) {}
		AnyType(uint64_t v) : value(v) {}
		AnyType(float v) : value(v) {}
		AnyType(const std::string& v) : value(v) {}
		AnyType(const char* v) : value(std::string(v ? v : "")) {}
		AnyType(const AnyType&) = default;
		AnyType(AnyType&&) noexcept = default;
		AnyType& operator=(const AnyType&) = default;
		AnyType& operator=(AnyType&&) noexcept = default;
		~AnyType() = default;

		MTDataType GetType() const { return std::visit([](const auto& v) { return model::GetType(v); }, value); }
		std::string ToString() const { return std::visit([](const auto& v) { return model::ToString(v); }, value); }
		const char* TypeName() const { return model::GetTypeName(GetType()); }
		template <typename T> T get() const
		{
			return std::visit([](const auto& stored) -> T {
				using S = std::decay_t<decltype(stored)>;
				if constexpr (std::is_same_v<T, S>) return stored;
				else if constexpr (std::is_arithmetic_v<T> && std::is_arithmetic_v<S>) return static_cast<T>(stored);
				else if constexpr (std::is_same_v<T, std::string>) return model::ToString(stored);
				else throw std::bad_variant_access();
			}, value);
		}
		template <typename T> void set(const T& v) { value = v; }
		void destroyData() { value = uint64_t(0); }
		bool operator<(const AnyType& rhs) const { return GetType() == rhs.GetType() && value < rhs.value; }
		bool operator<=(const AnyType& rhs) const { return GetType() == rhs.GetType() && value <= rhs.value; }
		bool operator>(const AnyType& rhs) const { return GetType() == rhs.GetType() && value > rhs.value; }
		bool operator>=(const AnyType& rhs) const { return GetType() == rhs.GetType() && value >= rhs.value; }
		bool operator==(const AnyType& rhs) const { return GetType() == rhs.GetType() && value == rhs.value; }
		bool operator!=(const AnyType& rhs) const { return GetType() != rhs.GetType() || value != rhs.value; }
	private:
		std::variant<bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float, std::string> value;
	};
}
