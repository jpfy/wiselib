
#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"

using namespace wiselib;

typedef OSMODEL Os;

#include "util/allocators/malloc_free_allocator.h"
typedef MallocFreeAllocator<Os> Allocator;

//#include "util/allocators/first_fit_allocator.h"
//typedef FirstFitAllocator<Os, 1024, 128> Allocator;
Allocator& get_allocator();

typedef Os::block_data_t block_data_t;
typedef Os::size_t size_type;
typedef uint16_t bitmask_t;

#include <util/broker/broker.h>
#include <util/broker/direct_broker_protocol.h>
#include <util/broker/direct_broker_protocol_command_message.h>
#include <util/broker/protobuf_rdf_serializer.h>
#include <util/broker/shdt_serializer.h>
#include <algorithms/codecs/huffman_codec.h>
#include <util/pstl/list_dynamic.h>
#include <util/pstl/vector_dynamic.h>
#include <util/pstl/vector_dynamic_set.h>
#include <util/tuple_store/codec_tuplestore.h>
#include <util/tuple_store/prescilla_dictionary.h>
#include <util/tuple_store/null_dictionary.h>
#include <util/tuple_store/tuplestore.h>

typedef BrokerTuple<Os, bitmask_t> BrokerTupleT;
typedef list_dynamic<Os, BrokerTupleT> TupleContainer;
// Prescilla
typedef PrescillaDictionary<Os> PrescillaDict;

#define COL(X) (1 << (X))
#define RDF_COLS (COL(0) | COL(1) | COL(2))

// TupleStore(s)

//typedef TupleStore<Os, TupleContainer, NullDictionary<Os>, Os::Debug, 0, &BrokerTupleT::compare> DictStore;
typedef TupleStore<Os, TupleContainer, PrescillaDict, Os::Debug, RDF_COLS, &BrokerTupleT::compare> DictStore;
typedef CodecTupleStore<Os, DictStore, HuffmanCodec<Os>, RDF_COLS> PresCodecDictStore;

// Broker

typedef uint16_t bitmask_t;
typedef Broker<Os, PresCodecDictStore, bitmask_t> broker_t;

// Protocols & Serializations

typedef DirectBrokerProtocol<Os, broker_t, Os::Radio, Os::Debug> B2B;
typedef ShdtSerializer<Os, 10> Shdt;
typedef ProtobufRdfSerializer<Os> Protobuf;

char* large_document[][3] = {
	#include "btcsample0.cpp"
};

typedef broker_t::Tuple Tuple;
typedef broker_t::TupleStore::column_mask_t column_mask_t;

class App {
	public:
		void init(Os::AppMainParameter& amp) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
			//radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet(amp);
			//timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp);
			
			dict.init(debug_);
			//ts.init(&dict, &container, debug_);
			codec_ts.init(&dict, &container, debug_);
			broker.init(&codec_ts);
			
			//broker.init(debug_);
			
			//insert_large_document();
			insert_small_documents();
			
			//retrieve_document("doc1");
			
			test_broker();
		}
		
		void ins(const char* s, const char *p, const char* o, bitmask_t bm) {
			typename broker_t::Tuple t;
			t.set(0, (block_data_t*)s);
			t.set(1, (block_data_t*)p);
			t.set(2, (block_data_t*)o);
			t.set_bitmask(bm);
			//broker.insert_tuple(t, bm);
			broker.tuple_store().insert(t);
		}
		
		void insert_large_document() {
			bitmask_t largemask = broker.create_document("large");
			for(size_t i=0; i < sizeof(large_document) / sizeof(large_document[0]); i++) {
				ins(large_document[i][0], large_document[i][1], large_document[i][2], largemask);
			}
		}
		
		bitmask_t doc1mask;
		bitmask_t doc2mask;
		bitmask_t doc3mask;	
		
		void insert_small_documents() {
			doc1mask = broker.create_document("doc1");
			doc2mask = broker.create_document("doc2");
			doc3mask = broker.create_document("doc3");
			ins("ab", "7777777", "dd", doc1mask );
			ins("ab", "88888888", "xx", doc1mask );
			ins("yy1", "999999999", "abx", doc1mask | doc3mask);
			ins("yy2", "999999999", "ab", doc1mask | doc3mask);
			ins("ab", "a1", "xx", doc1mask | doc2mask);
			ins("yy3", "999999999", "abx", doc1mask | doc3mask);
			ins("ab", "a2", "xx", doc1mask | doc2mask);
			ins("ab", "a3", "xx", doc1mask | doc2mask);
			ins("yy4", "999999999", "ab", doc1mask | doc3mask);
			ins("yy5", "999999999", "ab", doc1mask | doc3mask);
			ins("yy6", "999999999", "ab", doc1mask | doc3mask);
		}
		
		void test_broker() {
			//bitmask_t doc1mask = broker.create_document("doc1");
			//bitmask_t doc2mask = broker.create_document("doc2");
			//bitmask_t doc3mask = broker.create_document("doc3");
			//recvmask = broker.create_document("recv");
			
			debug_->debug("masks=%x %x %x\n", (int)doc1mask, (int)doc2mask, (int)doc3mask); // (int)recvmask);
			
			typename broker_t::Tuple t;
			broker_t::iterator iter;
			
			//ins("ab", "7777777", "dd", doc1mask );
			//ins("ab", "88888888", "xx", doc1mask | doc2mask);
			//ins("yy", "999999999", "ab", doc1mask | doc3mask);
			
			debug_->debug("--- iter doc1:");
			for(iter = broker.begin_document(doc1mask); iter != broker.end_document(doc1mask); ++iter) {
				broker_t::Tuple t = *iter;
				debug_->debug("(%s %s %s  mask=%x)", t.get(0), t.get(1), t.get(2), t.bitmask());
			}
			
			debug_->debug("--- iter doc2:");
			for(iter = broker.begin_document(doc2mask); iter != broker.end_document(doc2mask); ++iter) {
				broker_t::Tuple t = *iter;
				debug_->debug("(%s %s %s  mask=%x)", t.get(0), t.get(1), t.get(2), t.bitmask());
			}
			
			debug_->debug("--- iter doc3:");
			for(iter = broker.begin_document(doc3mask); iter != broker.end_document(doc3mask); ++iter) {
				broker_t::Tuple t = *iter;
				debug_->debug("(%s %s %s  mask=%x)", t.get(0), t.get(1), t.get(2), t.bitmask());
			}
			
			debug_->debug("--- now deleting all (ab ? ?) tuples");
			broker_t::Tuple query;
			query.set(0, (block_data_t*)"ab");
			broker_t::TupleStore::iterator it = broker.tuple_store().begin(&query, (1 << 0));
			
			while(it != broker.tuple_store().end()) {
				it = broker.tuple_store().erase(it);
			}
			
			debug_->debug("--- iter doc1:");
			for(broker_t::iterator iter = broker.begin_document(doc1mask); iter != broker.end_document(doc1mask); ++iter) {
				debug_->debug("(%s %s %s  mask=%x)", iter->get(0), iter->get(1), iter->get(2), iter->bitmask());
			}
			
			//query.set(0, (block_data_t*)"yy");
			//it = broker.tuple_store().begin(&query, (1 << 0));
			//broker.tuple_store().erase(it);
			
			//debug_->debug("--- iter doc1:\n");
			//for(broker_t::iterator iter = broker.begin_document(doc1mask); iter != broker.end_document(doc1mask); ++iter) {
				//debug_->debug("(%s %s %s  mask=%x)\n", iter->get(0), iter->get(1), iter->get(2), iter->bitmask());
			//}
		}
		
		void retrieve_document(char * docname) {
			bitmask_t mask = broker.get_document_mask(docname);
			debug_->debug("--- retrieve: %s\n", docname);
			for(broker_t::iterator iter = broker.begin_document(mask); iter != broker.end_document(mask); ++iter) {
				debug_->debug("(%s %s %s  mask=%x qrymask=%x)\n", iter->get(0), iter->get(1), iter->get(2), iter->bitmask(), iter.query().bitmask());
			}
		}
	
	private:
		DictStore::Dictionary dict;
		DictStore::TupleContainer container;
		//DictStore ts;
		PresCodecDictStore codec_ts;
		broker_t broker;
		bitmask_t recvmask;
		
		Os::Debug::self_pointer_t debug_;
};

wiselib::WiselibApplication<Os, App> app;

Allocator allocator_;

Allocator& get_allocator() { return allocator_; }

void application_main(Os::AppMainParameter& amp) {
	app.init(amp);
}

/* vim: set ts=4 sw=4 tw=78 noexpandtab foldmethod=marker foldenable :*/

