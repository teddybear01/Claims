/*
 * ids.h
 *	Suggested by Liyongfeng and Zhanglei, all the ids are defined in
 *	this file or better code organization purposes.
 *  Created on: Nov 8, 2013
 *      Author: wangli
 */

#ifndef IDS_H_
#define IDS_H_
#include <boost/unordered_map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <sstream>
typedef int NodeID;

typedef unsigned TableID;
typedef unsigned AttributeOffset;
typedef int ProjectionOffset;
typedef unsigned ColumnOffset;
typedef unsigned PartitionOffset;
typedef unsigned ChunkOffset;

/*the following ids are based on the assumption that the TableOffset is globally unique.*/


/**
 * AttributeID: an attribute in a table has an unique AttributeID*/
struct AttributeID{
	AttributeID(TableID tid,AttributeOffset off):table_id(tid),offset(off){};
	AttributeID(){};
	TableID table_id;
	AttributeOffset offset;
	bool operator==(const AttributeID& r)const{
		return table_id==r.table_id&&offset==r.offset;
	}
};
/* for boost::unordered_map*/
static size_t hash_value(const AttributeID& key){
	size_t seed=0;
	boost::hash_combine(seed,boost::hash_value(key.offset));
	boost::hash_combine(seed,boost::hash_value(key.table_id));
	return seed;
}


/**
 * ProjectionID: a projection has an unique ProjectionID
 */
struct ProjectionID{
	ProjectionID(){};
	ProjectionID(TableID tid,ProjectionOffset off):table_id(tid),projection_off(off){};
	ProjectionID(const ProjectionID& r):table_id(r.table_id),projection_off(r.projection_off){};
	TableID table_id;
	ProjectionOffset projection_off;
	bool operator==(const ProjectionID& r)const{
		return table_id==r.table_id&& projection_off==r.projection_off;
	}

	/* for boost::serialization*/
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
            ar & table_id & projection_off;
    }

};
/* for boost::unordered_map*/
static size_t hash_value(const ProjectionID& key){
	size_t seed=0;
	boost::hash_combine(seed,boost::hash_value(key.table_id));
	boost::hash_combine(seed,boost::hash_value(key.projection_off));
	return seed;
}


/**
 * ColumnID: a Column corresponds to an attribute and is physically stored in one projection.
 */
struct ColumnID{
	ColumnID(){};
	ColumnID(ProjectionID pid,ColumnOffset off):projection_id(pid),column_off(off){};
	ProjectionID projection_id;
	ColumnOffset column_off;
	bool operator==(const ColumnID &r)const{
		return projection_id==r.projection_id&&column_off==r.column_off;
	}
};
/* for boost::unordered_map*/
static size_t hash_value(const ColumnID& key){
	size_t seed=0;
	boost::hash_combine(seed,hash_value(key.projection_id));
	boost::hash_combine(seed,boost::hash_value(key.column_off));
	return seed;
}

/**
 * PartitionID: a partition corresponds to one projection.
 */
struct PartitionID{
	PartitionID(ProjectionID projection_id,PartitionOffset off):projection_id(projection_id),partition_off(off){};
	PartitionID(){};
	ProjectionID projection_id;
	PartitionOffset partition_off;
	bool operator==(const PartitionID& r)const{
		return projection_id==r.projection_id&&partition_off==r.partition_off;
	}
	PartitionID(const PartitionID& r){
		projection_id=r.projection_id;
		partition_off=r.partition_off;
	}
	std::string getName()const{
		std::ostringstream str;
		str<<"T"<<projection_id.table_id<<"G"<<projection_id.projection_off<<"P"<<partition_off;
		return str.str();
	}
	std::string getPathAndName()const{
		std::ostringstream str;
		str<<"/home/imdb/data/wangli/T"<<projection_id.table_id<<"G"<<projection_id.projection_off<<"P"<<partition_off;
//		str<<"/home/imdb/data/wangli/T"<<partition_off;
		return str.str();
	}
};
/* for boost::unordered_map*/
static size_t hash_value(const PartitionID& key){
	size_t seed=0;
	boost::hash_combine(seed,hash_value(key.projection_id));
	boost::hash_combine(seed,boost::hash_value(key.partition_off));
	return seed;
}

struct ChunkID{
	ChunkID(){};
	ChunkID(PartitionID partition_id,ChunkOffset chunk_offset):partition_id(partition_id),chunk_off(chunk_offset){};
	ChunkID(const ChunkID& r){
		partition_id=r.partition_id;
		chunk_off=r.chunk_off;
	}
	bool operator==(const ChunkID& r)const{
		return partition_id==r.partition_id&&partition_id==r.partition_id;
	}
	PartitionID partition_id;
	ChunkOffset chunk_off;
};
/* for boost::unordered_map*/
static size_t hash_value(const ChunkID& key){
	size_t seed=0;
	boost::hash_combine(seed,hash_value(key.partition_id));
	boost::hash_combine(seed,boost::hash_value(key.chunk_off));
	return seed;
}

#endif /* IDS_H_ */
