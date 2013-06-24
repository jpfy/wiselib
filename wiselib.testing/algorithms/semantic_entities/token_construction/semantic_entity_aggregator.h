/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/

#ifndef SEMANTIC_ENTITY_AGGREGATOR_H
#define SEMANTIC_ENTITY_AGGREGATOR_H

#include <external_interface/external_interface.h>

#include <util/pstl/set_vector.h>
#include <util/pstl/map_static_vector.h>
#include <util/broker/shdt_serializer.h>
#include "semantic_entity_id.h"
//#include "semantic_entity_aggregation_message.h"

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename TupleStore_P,
		typename Value_P
	>
	class SemanticEntityAggregator {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef TupleStore_P TupleStoreT;
			typedef Value_P Value;
			typedef ShdtSerializer<OsModel, 8> Shdt;
			typedef typename TupleStoreT::Dictionary DictionaryT;
			
			/*
			 * Matches TypeInfo in inqp/projection_info.h
			 */
			enum DataType { IGNORE = 0, INTEGER = 1, FLOAT = 2, STRING = 3 };
			
			enum Restrictions { MAX_ENTRIES = 8 };
			
			//typedef vector_static<OsModel, AggregationEntry, MAX_ENTRIES> AggregationEntries;
			
		private:
			class AggregationKey {
				public:
					AggregationKey() : uom_key_(DictionaryT::NULL_KEY), type_key_(DictionaryT::NULL_KEY) {
					}
					
					AggregationKey(const SemanticEntityId& se_id, typename DictionaryT::key_type sensor_type,
							typename DictionaryT::key_type uom, ::uint8_t datatype) :
						se_id_(se_id), datatype_(datatype), uom_key_(uom), type_key_(sensor_type) {
					}
					
					bool operator==(const AggregationKey& other) const {
						return (se_id_ == other.se_id_) &&
							(datatype_ == other.datatype_) &&
							(uom_key_ == other.uom_key_) &&
							(type_key_ == other.type_key_);
					}
					
					void free_dictionary_references(DictionaryT& dict, bool soft=false) {
						if(uom_key_ != DictionaryT::NULL_KEY) {
							dict.erase(uom_key_);
							if(!soft) { uom_key_ = DictionaryT::NULL_KEY; }
						}
						if(type_key_ != DictionaryT::NULL_KEY) {
							dict.erase(type_key_);
							if(!soft) { type_key_ = DictionaryT::NULL_KEY; }
						}
					}
					
					SemanticEntityId& se_id() { return se_id_; }
					void set_se_id(const SemanticEntityId& id) { se_id_ = id; }
					
					typename DictionaryT::key_type uom_key() { return uom_key_; }
					void set_uom_key(typename DictionaryT::key_type u) { uom_key_ = u; }
					
					typename DictionaryT::key_type type_key() { return type_key_; }
					void set_type_key(typename DictionaryT::key_type u) { type_key_ = u; }
					
					::uint8_t& datatype() { return datatype_; }
					void set_datatype(::uint8_t d) { datatype_ = d; }
					
				private:
					SemanticEntityId se_id_;
					::uint8_t datatype_;
					typename DictionaryT::key_type uom_key_;
					typename DictionaryT::key_type type_key_;
			};
			
			class AggregationValue {
				public:
					AggregationValue() : count_(0) {
					}
					
					void init(Value v) {
						total_count_ = count_ = 1;
						total_min_ = total_max_ = total_mean_ = min_ = max_ = mean_ = v;
					}
					
					void aggregate(Value v, ::uint8_t datatype) {
						DBG("aggr %ld before %2d/%2d/%2d",
								(long)v, (int)min_, (int)max_, (int)mean_);
						
						if(datatype == INTEGER) {
							if(v < min_) { min_ = v; }
							if(v > max_) { max_ = v; }
							++count_;
							mean_ += (v - mean_) / count_;
						}
						else {
							assert(false && "not supported!");
						}
						
						DBG("aggr %ld after %2d/%2d/%2d",
								(long)v, (int)min_, (int)max_, (int)mean_);
					}
					
					Value& count() { return count_; }
					void set_count(Value& x) { count_ = x; }
					Value& min() { return min_; }
					void set_min(Value& x) { min_ = x; }
					Value& max() { return max_; }
					void set_max(Value& x) { max_ = x; }
					Value& mean() { return mean_; }
					void set_mean(Value& x) { mean_ = x; }
					
					Value& total_count() { return total_count_; }
					void set_total_count(Value& x) { total_count_ = x; }
					Value& total_min() { return total_min_; }
					void set_total_min(Value& x) { total_min_ = x; }
					Value& total_max() { return total_max_; }
					void set_total_max(Value& x) { total_max_ = x; }
					Value& total_mean() { return total_mean_; }
					void set_total_mean(Value& x) { total_mean_ = x; }
					
					void set_totals() {
						total_count_ = count_;
						total_min_ = min_;
						total_max_ = max_;
						total_mean_ = mean_;
						
						count_ = min_ = max_ = mean_ = 0;
					}
					
				private:
					Value count_, min_, max_, mean_;
					Value total_count_, total_min_, total_max_, total_mean_;
			};
			
			enum Fields {
				FIELD_UOM = 0,
				FIELD_TYPE, FIELD_DATATYPE, FIELD_COUNT, FIELD_MIN, FIELD_MAX, FIELD_MEAN,
				FIELD_TOTAL_COUNT, FIELD_TOTAL_MIN, FIELD_TOTAL_MAX, FIELD_TOTAL_MEAN
			};
		
		public:
			typedef MapStaticVector<OsModel, AggregationKey, AggregationValue, MAX_ENTRIES> AggregationEntries;
			typedef typename AggregationEntries::iterator iterator;
			
			void init(typename TupleStoreT::self_pointer_t ts) { //, const char* entity_format) {
				tuple_store_ = ts;
				//entity_format_ = entity_format;
			}
			
			void aggregate(const SemanticEntityId& se_id, Value sensor_type, Value uom, Value value, ::uint8_t datatype) {
				AggregationKey k(se_id, sensor_type, uom, datatype);
				if(aggregation_entries_.contains(k)) {
					AggregationValue& v = aggregation_entries_[k];
					v.aggregate(value, datatype);
				}
				else {
					//DBG("k not found, contains:");
					//for(iterator iter=begin(); iter!=end(); ++iter) {
						//DBG("contains %d.%08lx
					
					aggregation_entries_[k].init(value);
				}
			}
			
			/**
			 * For the given SE, move the current aggregation values to the
			 * total values, then reset the current values to 0.
			 */
			void set_totals(const SemanticEntityId& se_id) {
				for(iterator iter = begin(); iter != end(); ++iter) {
					if(iter->first.se_id() != se_id) { continue; }
					iter->second.set_totals();
				}
			}
			
			iterator begin() { return aggregation_entries_.begin(); }
			iterator end() { return aggregation_entries_.end(); }
			
			/**
			 * @param call_again set to true if not all data has been written
			 * (call this repeadetely until call_again is false!)
			 * @return number of bytes written
			 */
			size_type fill_buffer_start(const SemanticEntityId& se_id, block_data_t* buffer, size_type buffer_size, bool& call_again) {
				fill_buffer_iterator_ = aggregation_entries_.begin();
				fill_buffer_state_ = FIELD_UOM;
				return fill_buffer(se_id, buffer, buffer_size, call_again);
			}
			
			/// ditto.
			size_type fill_buffer(const SemanticEntityId& se_id, block_data_t* buffer, size_type buffer_size, bool& call_again) {
				//{{{
				
				if(fill_buffer_iterator_ == aggregation_entries_.end()) {
					call_again = false;
					return 0;
				}
				
				call_again = false;
				block_data_t *buf = buffer, *buf_end = buffer + buffer_size;
				bool run = true;
				
				while(!call_again && buf_end - buf && run) {
					AggregationKey& key = fill_buffer_iterator_->first;
					AggregationValue& aggregate = fill_buffer_iterator_->second;
					
					switch(fill_buffer_state_) {
						case FIELD_UOM: {
							block_data_t *uom = dictionary().get_value(key.uom_key());
							call_again = shdt_.write_field(FIELD_UOM, uom, strlen((char*)uom), buf, buf_end);
							dictionary().free_value(uom);
							if(!call_again) { fill_buffer_state_ = FIELD_TYPE; }
							break;
						}
						
						case FIELD_TYPE: {
							block_data_t *type = dictionary().get_value(key.type_key());
							call_again = shdt_.write_field(FIELD_TYPE, type, strlen((char*)type), buf, buf_end);
							dictionary().free_value(type);
							if(!call_again) { fill_buffer_state_ = FIELD_DATATYPE; }
							break;
						}
						
						case FIELD_DATATYPE:
							call_again = shdt_.write_field(FIELD_DATATYPE, &key.datatype(), sizeof(key.datatype()), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_COUNT; }
							break;
							
						case FIELD_COUNT:
							call_again = shdt_.write_field(FIELD_COUNT, (block_data_t*)&aggregate.count(), sizeof(aggregate.count()), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_MIN; }
							break;
							
						case FIELD_MIN:
							call_again = shdt_.write_field(FIELD_MIN, (block_data_t*)&aggregate.min(), sizeof(aggregate.min()), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_MAX; }
							break;
							
						case FIELD_MAX:
							call_again = shdt_.write_field(FIELD_MAX, (block_data_t*)&aggregate.max(), sizeof(aggregate.max()), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_MEAN; }
							break;
							
						case FIELD_MEAN:
							call_again = shdt_.write_field(FIELD_MEAN, (block_data_t*)&aggregate.mean(), sizeof(aggregate.mean()), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_TOTAL_COUNT; }
							break;
							
						case FIELD_TOTAL_COUNT:
							call_again = shdt_.write_field(FIELD_TOTAL_COUNT, (block_data_t*)&aggregate.total_count(), sizeof(aggregate.total_count()), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_TOTAL_MIN; }
							break;
							
						case FIELD_TOTAL_MIN:
							call_again = shdt_.write_field(FIELD_TOTAL_MIN, (block_data_t*)&aggregate.total_min(), sizeof(aggregate.total_min()), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_TOTAL_MAX; }
							break;
							
						case FIELD_TOTAL_MAX:
							call_again = shdt_.write_field(FIELD_TOTAL_MAX, (block_data_t*)&aggregate.total_max(), sizeof(aggregate.total_max()), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_TOTAL_MEAN; }
							break;
							
						case FIELD_TOTAL_MEAN:
							call_again = shdt_.write_field(FIELD_TOTAL_MEAN, (block_data_t*)&aggregate.total_mean(), sizeof(aggregate.total_mean()), buf, buf_end);
							if(!call_again) {
								++fill_buffer_iterator_;
								if(fill_buffer_iterator_ == aggregation_entries_.end()) {
									run = false;
								}
							}
							break;
					} // switch()
				} // while()
				
				return buf - buffer;
				
				//}}}
			} // fill_buffer()
			
			/**
			 */
			void read_buffer(const SemanticEntityId& id, block_data_t* buffer, size_type buffer_size) {
				//{{{
				
				typename Shdt::Reader reader(&shdt_, buffer, buffer_size);
				bool done = true;
				block_data_t *data;
				size_type data_size;
				typename Shdt::field_id_t field_id;
				
				DBG("aggr this is read_buffer buffer_size=%d", buffer_size);
				
				while(!reader.done()) {
					done = reader.read_field(field_id, data, data_size);
					DBG("aggr read_buffer done=%d", done);
					
					Value& v = reinterpret_cast<Value&>(*data);
					if(!done) { break; }
					DBG("aggr read_buffer field id %d", field_id);
					switch(field_id) {
						case FIELD_UOM: {
							typename DictionaryT::key_type k = dictionary().insert(data);
							read_buffer_key_.set_uom_key(k);
							break;
						}
						case FIELD_TYPE: {
							typename DictionaryT::key_type k = dictionary().insert(data);
							read_buffer_key_.set_type_key(k);
							break;
						}
						case FIELD_DATATYPE:
							read_buffer_key_.set_datatype(*data);
							break;
							
						case FIELD_COUNT:
							read_buffer_value_.set_count(v);
							break;
						case FIELD_MIN:
							read_buffer_value_.set_min(v);
							break;
						case FIELD_MAX:
							read_buffer_value_.set_max(v);
							break;
						case FIELD_MEAN:
							read_buffer_value_.set_mean(v);
							break;
							
						case FIELD_TOTAL_COUNT:
							read_buffer_value_.set_total_count(v);
							break;
						case FIELD_TOTAL_MIN:
							read_buffer_value_.set_total_min(v);
							break;
						case FIELD_TOTAL_MAX:
							read_buffer_value_.set_total_max(v);
							break;
						case FIELD_TOTAL_MEAN: {
							read_buffer_value_.set_total_mean(v);
							
							read_buffer_key_.set_se_id(id);
							
							if(aggregation_entries_.contains(read_buffer_key_)) {
								// we are going to insert that key again, that
								// would increase the number of references to
								// type/uom strings without increasing the
								// number of keys that reference it.
								// Thus, free one set of references here to
								// compensate.
								read_buffer_key_.free_dictionary_references(dictionary(), true);
							}
							
							aggregation_entries_[read_buffer_key_] = read_buffer_value_;
							break;
						}
					} // switch
				} // while
				//}}}
			}
			
			DictionaryT& dictionary() { return tuple_store_->dictionary(); }
			
		private:
			
			//size_type find_entry(const SemanticEntityId& se_id, Value sensor_type, Value uom, ::uint8_t datatype) {
			//}
			
			int fill_buffer_state_;
			const char* entity_format_;
			typename TupleStoreT::self_pointer_t tuple_store_;
			AggregationEntries aggregation_entries_;
			typename AggregationEntries::iterator fill_buffer_iterator_;
			Shdt shdt_;
			AggregationKey read_buffer_key_;
			AggregationValue read_buffer_value_;
		
	}; // SemanticEntityAggregator
	
	/*
	template<
		typename OsModel_P,
		typename QueryProcessor_P
	>
	const char* SemanticEntityAggregator<OsModel_P, QueryProcessor_P>::entity_format_ = "<http://www.spitfire-project.eu/se/%04x.%04x>";
	*/
}

#endif // SEMANTIC_ENTITY_AGGREGATOR_H

