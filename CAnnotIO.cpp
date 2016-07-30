/* 
 * File:			CMghIO.cpp
 * Description:		A class for reading and writing '.annot' file of FreeSurfer
 * Author:		Zhang Teng, PhD Candidate
 * Organization:		Department of Imaging and Interventional Radiology, The Chinese University of Hong Kong
 * Mailbox:		zhangteng630@gmail.com
 * License:		GPL, http://www.gnu.org/licenses/gpl-3.0.en.html
 * 
 * Created on January 12, 2016, 00:44 AM
 * Modified on July 25, 2016, 2:31 PM
 */

#include <map>

#include "CAnnotIO.h"

CAnnotIO::CAnnotIO()
{
}

CAnnotIO::~CAnnotIO()
{
}

int CAnnotIO::Read(std::string filename)
{
	//open binary file
	std::ifstream ifs(filename.c_str(), std::ios::binary | std::ios::in);
	if (!ifs.is_open())
	{
		std::cerr << "**Error opening annotation file " << filename << std::endl;
		return 1;
	}
	//read vertex count
	m_vertexCount = ReadBigEndian<int32_t>(ifs);
	//read vertices
	m_vertices.clear();
	m_vertices.resize(m_vertexCount);
	m_labels.clear();
	m_labels.resize(m_vertexCount);
	for (int i = 0; i < m_vertexCount; ++i)
	{
		if (ifs.eof())
		{
			std::cerr << "**Error: broken annotation file " << filename << std::endl;
			return 1;
		}
		m_vertices[i] = ReadBigEndian<int32_t>(ifs);
		m_labels[i] = ReadBigEndian<int32_t>(ifs);
	}
	if (ifs.eof())
	{
		ifs.close();
		std::cout << "**Warning: no color tables in annotation file " << filename << std::endl;
		return 1;
	}
	//read the tag "TAG_OLD_COLORTABLE" which indicates a color table comes next
	ReadBigEndian<int32_t>(ifs);
	//read number of entries (1. also called label number); (2. also a feature to differentiate version 1 and 2)
	int32_t numEntries = ReadBigEndian<int32_t>(ifs);
	m_colortable.clear();
	//the original version
	if (numEntries > 0)
	{
		std::cout << "Reading from Original Version" << std::endl;
		//read file name
		int32_t len = ReadBigEndian<int32_t>(ifs);
		char * filename = static_cast<char*>(malloc(len*sizeof(char)));
		ifs.read(filename, len);
		//read color table
		for (int32_t i = 0; i < numEntries; ++i)
		{
			ColorTableItem item;
			//read structure name
			len = ReadBigEndian<int32_t>(ifs);
			char * struct_name = static_cast<char*>(malloc(len*sizeof(char)));
			ifs.read(struct_name, len);
			item.name = std::string(struct_name);
			//read red
			item.r = ReadBigEndian<int32_t>(ifs);
			//read green
			item.g = ReadBigEndian<int32_t>(ifs);
			//read blue
			item.b = ReadBigEndian<int32_t>(ifs);
			//read transparency
			item.t = ReadBigEndian<int32_t>(ifs);
			//calculate label
			item.label = item.r + item.g * 256 + item.b * 256 * 256 + item.t * 256 * 256 * 256;
			m_colortable.insert(std::pair< int32_t, ColorTableItem >(item.label, item));
		}
	}
	//the second version
	else
	{
		int32_t version = -numEntries;
		if (version != 2)
		{
			std::cerr << "Error! Does not handle version " << version << std::endl;
			return 1;
		}
		//std::cout << "Reading from version " << version << std::endl;
		//read real label number
		numEntries = ReadBigEndian<int32_t>(ifs);
		//read file name
		int32_t len = ReadBigEndian<int32_t>(ifs);
		char * filename = static_cast<char*>(malloc(len*sizeof(char)));
		ifs.read(filename, len);
		//read label number to be read
		int32_t numEntriesToRead = ReadBigEndian<int32_t>(ifs);
		std::vector< int32_t > structureIds;
		for (int32_t i = 0; i < numEntriesToRead; ++i)
		{
			//read structure id
			int32_t structureId = ReadBigEndian<int32_t>(ifs);
			if (structureId < 0)
			{
				std::cerr << "Error! Read entry, index " << structureId << std::endl;
				return 1;
			}
			if (std::find(structureIds.begin(), structureIds.end(), structureId) != structureIds.end())
			{
				std::cerr << "Error! Duplicated Structure " << structureId << std::endl;
				return 1;
			}			
			ColorTableItem item;
			//read structure name
			len = ReadBigEndian<int32_t>(ifs);
			char * struct_name = static_cast<char*>(malloc(len*sizeof(char)));
			ifs.read(struct_name, len);	
			item.name = std::string(struct_name);
			//read red
			item.r = ReadBigEndian<int32_t>(ifs);
			//read green
			item.g = ReadBigEndian<int32_t>(ifs);
			//read blue
			item.b = ReadBigEndian<int32_t>(ifs);
			//read transparency
			item.t = ReadBigEndian<int32_t>(ifs);
			//calculate label
			item.label = item.r + item.g * 256 + item.b * 256 * 256 + item.t * 256 * 256 * 256;
			m_colortable.insert(std::pair< int32_t, ColorTableItem >(item.label, item));
		}
	}
	return 0;
}

int CAnnotIO::Write(std::string filename)
{
	std::ofstream ofs(filename.c_str(), std::ios::binary | std::ios::out);
	if(!ofs.is_open())
	{
		std::cerr << "**Error opening annotation file " << filename << std::endl;
		return 1;
	}
	//write vertex count
	WriteBigEndian<int32_t>(ofs, m_vertexCount);
	//write vertex and label
	for(int32_t i = 0; i < m_vertexCount; ++i)
	{
		WriteBigEndian<int32_t>(ofs, m_vertices[i]);
		WriteBigEndian<int32_t>(ofs, m_labels[i]);
	}
	//write tag indicating a color table comes next
	WriteBigEndian<int32_t>(ofs, 0);
	//version 2
	WriteBigEndian<int32_t>(ofs, -2);
	//write number of entries
	WriteBigEndian<int32_t>(ofs, m_colortable.size());
	//write file name
	WriteBigEndian<int32_t>(ofs, filename.size());
	WriteBigEndian<char>(ofs, *(filename.c_str()));
	//write number to be read/write
	WriteBigEndian<int32_t>(ofs, m_colortable.size());
	int32_t id = 0;
	for (std::map< int32_t, ColorTableItem >::iterator iter = m_colortable.begin();
		iter != m_colortable.end(); ++iter)
	{
		//structure id
		WriteBigEndian<int32_t>(ofs, id++);
		ColorTableItem item = (*iter).second;
		//structure name
		WriteBigEndian<int32_t>(ofs, item.name.size());
		WriteBigEndian<char>(ofs, *(item.name.c_str()));
		//write red
		WriteBigEndian<int32_t>(ofs, item.r);
		//write green
		WriteBigEndian<int32_t>(ofs, item.g);
		//write blue
		WriteBigEndian<int32_t>(ofs, item.b);
		//write transparency
		WriteBigEndian<int32_t>(ofs, item.t);
		//write label
		WriteBigEndian<int32_t>(ofs, item.label);
	}
	return 0;
}
