/* 
 * File:			CMghIO.h
 * Description:		A class for reading and writing '.annot' file of FreeSurfer
 * Author:		Zhang Teng, PhD Candidate
 * Organization:		Department of Imaging and Interventional Radiology, The Chinese University of Hong Kong
 * Mailbox:		zhangteng630@gmail.com
 * License:		GPL, http://www.gnu.org/licenses/gpl-3.0.en.html
 * 
 * Created on January 12, 2016, 00:44 AM
 * Modified on July 25, 2016, 2:31 PM
 */

#ifndef __CAnnotIO_h__
#define	__CAnnotIO_h__

#include <iostream>
#include <fstream>
#include <string>
#include <stdint.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>

class CAnnotIO
{
public:
	typedef struct {
		int32_t label;		//label
		std::string name;	//name
		int32_t r;		//red
		int32_t g;		//green
		int32_t b;		//blue
		int32_t t;		//transparency
	} ColorTableItem;
	
public:
	CAnnotIO();
	~CAnnotIO();

	//read .annot file
	int Read(std::string filename);
	
	//write .annot file
	int Write(std::string filename);
	
	//get access to m_vertexCount
	int32_t& GetVertexCount(void){
		return m_vertexCount;
	}
	
	//get access to m_vertices
	std::vector< int32_t >& GetVertices(void){
		return m_vertices;
	}
	
	//get access to labels
	std::vector< int32_t >& GetLabels(void){
		return m_labels;
	}
	
	//get access to colortable
	std::map< int32_t, ColorTableItem >& GetColorTable(void){
		return m_colortable;
	}

public:
	//template function, to write big-endian coded data
	template< class _T >
	_T ReadBigEndian(std::ifstream& ifs){
		_T data;
		memset(&data, 0, sizeof(_T));
		for (int i = 0; i < sizeof(_T); ++i)
		{
			char byte;
			ifs.read(&byte, 1);
			memset((char*)(&data) + sizeof(_T) - i - 1, byte, 1);
		}
		return data;
	}
	
	//template function to write big-endian coded data
	template< class _T >
	void WriteBigEndian(std::ofstream& ofs, _T data){
		for(int i = sizeof(_T)-1; i >= 0; --i)
		{
			char byte;
			memset(&byte, *((char*)(&data)+i), 1);
			ofs.write(&byte, 1);
		}
	}

private:
	//vertex count
	int32_t m_vertexCount;
	
	//vertex number, (0 .. m_vertexCount - 1) by default
	std::vector< int32_t > m_vertices;
	
	//labels of corresponding vertex
	std::vector< int32_t > m_labels;
	
	std::map< int32_t, ColorTableItem > m_colortable;
};

#endif	/* __CAnnotIO_h__ */

