// Copyright 2013,2014 Matheus Marzano

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "Element.h"

Element::Element(const unsigned int idParam, const unsigned int noiseParam): id(idParam), noise(noiseParam) 
{
}

const bool Element::operator<(const Element& otherElement) const
{
  return noise < otherElement.noise;
}

const unsigned int Element::getId() const
{
  return id;
}

const unsigned int Element::getNoise() const
{
  return noise;
}

void Element::addNoise(const unsigned int n)
{
  noise += n;
}

const bool Element::smallerId(const Element& e1, const Element& e2)
{
  return e1.id < e2.id;
}

vector<unsigned int> Element::idVectorUnion(const vector<Element>& v1,const vector<Element>& v2)
{
  vector<unsigned int> unionVector;
  unionVector.reserve(v1.size() + v2.size());
  vector<Element>::const_iterator v1It = v1.begin();
  vector<Element>::const_iterator v2It = v2.begin();
  while (true)
    {
      if (v1It->id < v2It->id)
	{
	  unionVector.push_back(v1It->id);
	  if (++v1It == v1.end())
	    {
	      for (; v2It != v2.end(); ++v2It)
		{
		  unionVector.push_back(v2It->id);
		}
	      return unionVector;
	    }
	}
      else
	{
	  if (v1It->id == v2It->id)
	    {
	      if (++v1It == v1.end())
		{
		  for (; v2It != v2.end(); ++v2It)
		    {
		      unionVector.push_back(v2It->id);
		    }
		  return unionVector;
		}
	      unionVector.push_back(v2It->id);
	      if (++v2It == v2.end())
		{
		  for (; v1It != v1.end(); ++v1It)
		    {
		      unionVector.push_back(v1It->id);
		    }
		  return unionVector;
		}
	    }	
	  else
	    {
	      unionVector.push_back(v2It->id);
	      if (++v2It == v2.end())
		{
		  for (; v1It != v1.end(); ++v1It)
		    {
		      unionVector.push_back(v1It->id);
		    }
		  return unionVector;
		}
	    }
	}
    }
  return unionVector;
}
