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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_UTIL_LQI_DISTANCE_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_UTIL_LQI_DISTANCE_H

#include "internal_interface/routing_table/routing_table_static_array.h"
#include "util/pstl/map_static_vector.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"


//#define DISTANCE_DEBUG_

namespace wiselib
{

   template<typename OsModel_P, typename Debug_P , typename Arithmatic_P,
            typename Radio_P = typename OsModel_P::TxRadio,
            int TABLE_SIZE = 60
            >
   class LQIDistanceModel
   {
   public:

	#define LQI_TABLE_SIZE 60



      typedef OsModel_P OsModel;



      typedef Radio_P Radio;

      typedef Debug_P Debug;



      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::ExtendedData ExtendedData;
      typedef Arithmatic_P Arithmatic;

      typedef LQIDistanceModel<OsModel,  Debug, Arithmatic, Radio,TABLE_SIZE> self_type;
      typedef self_type* self_pointer_t;

      //typedef double Arithmatic;

      typedef uint8_t lqi_t;
      typedef StaticArrayRoutingTable<OsModel, Radio, TABLE_SIZE, Arithmatic> DistanceMap;
      typedef MapStaticVector<OsModel_P, lqi_t , Arithmatic, LQI_TABLE_SIZE> LQIMap;
      typedef typename DistanceMap::iterator DistanceMapIterator;
      typedef typename LQIMap::iterator LQIMapIterator;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      LQIDistanceModel(  )

      {

    	  ready = 0;
      }

      ~LQIDistanceModel(  )

      {


      }
      // --------------------------------------------------------------------
      int init( Radio* radio, Debug* debug, LQIMap* lqimap)
      {

    	  debug_ = debug;
    	  ready = 1;
#ifdef DISTANCE_DEBUG_

            debug_->debug("distance init");
#endif
         radio_ = radio;
         radio_->template reg_recv_callback<self_type, &self_type::receive>( this );

         lqis_ = lqimap;


 		LQIMapIterator previt =lqis_->begin();
#ifdef DISTANCE_DEBUG_
 		for ( LQIMapIterator it = lqis_->begin(); it != lqis_->end(); ++it )
 		{


 		  debug_->debug(" lqis is %d  %f\n",it->first,it->second);
 		}
#endif
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      Arithmatic distance( node_id_t to )
      {


    	  if(ready == 0){
#ifdef DISTANCE_DEBUG_
    		  debug_->debug(" distance to ::not init ");
#endif

    		  return -1;

    	  };

    	//  const double UNKNOWN_DISTANCE = DBL_MIN;
         Arithmatic distance = -1;

    	 // Arithmatic distance = 250;

#ifdef DISTANCE_DEBUG_
 		for ( DistanceMapIterator it2 = distances_.begin(); it2 != distances_.end(); ++it2 )
 		{


 		  debug_->debug(" distancess is %x  %f\n",it2->first,it2->second);
 		}
#endif
#ifdef DISTANCE_DEBUG_
 	//	debug_->debug(" distancess size is %d\n",distances_.size());
#endif
         DistanceMapIterator it = distances_.find( to );




#ifdef DISTANCE_DEBUG_
         debug_->debug("distance found %f oder %f ",it->second ,distances_[to]);
#endif

         if ( (it != distances_.end()) /*&& (it->second!=0)*/){
            distance = it->second;
#ifdef DISTANCE_DEBUG_

            debug_->debug("distance found %f from %x to %x",distance,radio_->id(), to);
#endif
         }

         return distance;
      };




   private:



      void receive( node_id_t id, size_t len, block_data_t* data, const ExtendedData& exdata)
           {
              // TODO: Why does the lqi-radio returns (255 - lqi)? hence, we transform it back here...
              lqi_t lqi = 255 - exdata.link_metric();

        	  if(ready == 0){
//#ifdef DISTANCE_DEBUG_
        		 // debug_->debug("receive ::not init ");
//#endif

        		  return;

        	  };

        	  ready = 0;

              if(lqi > 160)
            	  lqi = 151;

              if(lqi < 5)
            	  lqi = 5;

#ifdef DISTANCE_DEBUG_

           debug_->debug(" lqi = %d",lqi);
#endif

              Arithmatic distance = -1;

              LQIMapIterator previt =lqis_->begin();

             // debug_->debug(" begin value %d %f ",previt->first,previt->second);

              for (LQIMapIterator it = lqis_->begin(); it != lqis_->end(); ++it )
                      {

                         if ( it->first <= lqi ){
                        	 //distance = it->second + previt->second;
                        	 //distances_[id] = distance/2;


                        	 if(previt->first == it->first)
                        		 distances_[id]=it->second;
                        	 else
                        		 distances_[id] = previt->second + ( ((it->second - previt->second)/(previt->first - it->first)) * (previt->first - lqi));


#ifdef DISTANCE_DEBUG_

                        	 debug_->debug(" lqi found = %d = distance %f between %x from %x",lqi,distances_[id],radio_->id(),id);

#endif
                        	 ready = 1;
                        	 return;
                         }

                         previt = it;
                   	  //#ifdef DISTANCE_DEBUG_

                   	      //      debug_->debug(" lqi is %d  %f\n",it->first,it->second);
                   	  //#endif
                      }


#ifdef DISTANCE_DEBUG_
              	  if(distance == -1)
                       debug_->debug(" lqi not found \n",lqi,distance);
#endif


//#ifdef DISTANCE_DEBUG_

                  //     debug_->debug(" distance is %f \n",distances_[id]);
//#endif


//              distances_[id] = distance;
#ifdef DISTANCE_DEBUG_

            debug_->debug("distance received %f  %f ",distances_[id],distance);
#endif

            ready = 1;

           }








      Radio *radio_;
      Debug *debug_;
      LQIMap *lqis_;
      DistanceMap distances_;
      uint8_t ready;



   };




/*   template<typename OsModel_P, typename Debug_P ,
               typename Radio_P = typename OsModel_P::TxRadio,
               int TABLE_SIZE = 40>
   LQIDistanceModel<OsModel_P,  Debug_P   >::DistanceMap
   LQIDistanceModel<OsModel_P,  Debug_P  >::distances_ = null;
*/




}

#endif
