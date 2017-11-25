#include <common/serialization.h>

#include "Helpers.h"

#include <strmd/strmd/serializer.h>
#include <strmd/strmd/deserializer.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace micro_profiler
{
	namespace tests
	{
		namespace
		{
			class vector_adapter
			{
			public:
				vector_adapter()
					: _ptr(0)
				{	}

				void write(const void *buffer, size_t size)
				{
					const unsigned char *b = reinterpret_cast<const unsigned char *>(buffer);

					_buffer.insert(_buffer.end(), b, b + size);
				}

				void read(void *buffer, size_t size)
				{
					assert_is_true(size <= _buffer.size() - _ptr);
					memcpy(buffer, &_buffer[_ptr], size);
					_ptr += size;
				}

			private:
				size_t _ptr;
				vector<unsigned char> _buffer;
			};
		}

		begin_test_suite( SerializationHelpersTests )
			test( SerializedStatisticsAreDeserialized )
			{
				// INIT
				vector_adapter buffer;
				strmd::serializer<vector_adapter> s(buffer);
				function_statistics s1(17, 2012, 123123123, 32123, 2213), s2(1117, 212, 1231123, 3213, 112213);
				function_statistics ds1, ds2;

				// ACT (serialization)
				s(s1);
				s(s2);

				// INIT
				strmd::deserializer<vector_adapter> ds(buffer);

				// ACT (deserialization)
				ds(ds1);
				ds(ds2);

				// ASSERT
				assert_equal(s1, ds1);
				assert_equal(s2, ds2);
			}

			
			test( SerializationDeserializationOfContainersOfFunctionStaticsProducesTheSameResults )
			{
				// INIT
				vector_adapter buffer;
				strmd::serializer<vector_adapter> s(buffer);
				statistics_map s1, s2;
				statistics_map ds1, ds2;

				s1[(void*)123441] = function_statistics(17, 2012, 123123123, 32123, 2213);
				s1[(void*)71341] = function_statistics(1117, 212, 1231123, 3213, 112213);
				s2[(void*)141] = function_statistics(17, 12012, 1123123123, 132123, 12213);
				s2[(void*)7341] = function_statistics(21117, 2212, 21231123, 23213, 2112213);
				s2[(void*)7741] = function_statistics(31117, 3212, 31231123, 33213, 3112213);

				// ACT
				s(s1);
				s(s2);

				// INIT
				strmd::deserializer<vector_adapter> ds(buffer);

				// ACT (deserialization)
				ds(ds1);
				ds(ds2);

				// ASSERT
				assert_equivalent(mkvector(ds1), mkvector(s1));
				assert_equivalent(mkvector(ds2), mkvector(s2));
			}

			
			test( DeserializationIntoExistingValuesAddsValues )
			{
				// INIT
				vector_adapter buffer;
				strmd::serializer<vector_adapter> s(buffer);
				statistics_map s1, s2;

				s1[(void*)123441] = function_statistics(17, 2012, 123123123, 32123, 2213);
				s1[(void*)7741] = function_statistics(1117, 212, 1231123, 3213, 112213);
				s2[(void*)141] = function_statistics(17, 12012, 11293123, 132123, 12213);
				s2[(void*)7341] = function_statistics(21117, 2212, 21231123, 23213, 2112213);
				s2[(void*)7741] = function_statistics(31117, 3212, 31231123, 33213, 3112213);
				s(s1);
				s(s2);

				strmd::deserializer<vector_adapter> ds(buffer);

				// ACT
				ds(s2);

				// ASSERT
				statistics_map reference1;

				reference1[(void*)141] = function_statistics(17, 12012, 11293123, 132123, 12213);
				reference1[(void*)7341] = function_statistics(21117, 2212, 21231123, 23213, 2112213);
				reference1[(void*)7741] = function_statistics(31117 + 1117, 3212, 31231123 + 1231123, 33213 + 3213, 3112213);
				reference1[(void*)123441] = function_statistics(17, 2012, 123123123, 32123, 2213);

				assert_equivalent(mkvector(reference1), mkvector(s2));

				// ACT
				ds(s2);

				// ASSERT
				statistics_map reference2;

				reference2[(void*)141] = function_statistics(2 * 17, 12012, 2 * 11293123, 2 * 132123, 12213);
				reference2[(void*)7341] = function_statistics(2 * 21117, 2212, 2 * 21231123, 2 * 23213, 2112213);
				reference2[(void*)7741] = function_statistics(2 * 31117 + 1117, 3212, 2 * 31231123 + 1231123, 2 * 33213 + 3213, 3112213);
				reference2[(void*)123441] = function_statistics(17, 2012, 123123123, 32123, 2213);

				assert_equivalent(mkvector(reference2), mkvector(s2));
			}


			test( DetailedStatisticsIsSerializedAsExpected )
			{
				// INIT
				vector_adapter buffer;
				strmd::serializer<vector_adapter> s(buffer);
				function_statistics_detailed s1;

				static_cast<function_statistics &>(s1) = function_statistics(17, 2012, 123123123, 32123, 2213);
				s1.callees[(void*)7741] = function_statistics(1117, 212, 1231123, 3213, 112213);
				s1.callees[(void*)141] = function_statistics(17, 12012, 11293123, 132123, 12213);
				s1.callers[(void*)14100] = 1232;
				s1.callers[(void*)14101] = 12322;
				s1.callers[(void*)141000] = 123221;

				// ACT
				s(s1);

				// INIT
				strmd::deserializer<vector_adapter> ds(buffer);
				function_statistics_detailed ds1;
				vector< pair<const void *, function_statistics> > callees;
				vector< pair<const void *, unsigned long long> > callers;

				// ACT
				ds(ds1);

				// ASSERT
				pair<const void *, function_statistics> reference_callees[] = {
					make_pair((void*)7741, function_statistics(1117, 212, 1231123, 3213, 112213)),
					make_pair((void*)141, function_statistics(17, 12012, 11293123, 132123, 12213)),
				};
				pair<const void *, unsigned long long> reference_callers[] = {
					make_pair((void*)14100, 1232),
					make_pair((void*)14101, 12322),
					make_pair((void*)141000, 123221),
				};

				assert_equal(s1, ds1);
				assert_equivalent(reference_callees, mkvector(ds1.callees));
				assert_equivalent(reference_callers, mkvector(ds1.callers));
			}
		end_test_suite
	}
}