// Copyright 2013,2014 Matheus Marzano

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "Element.h"

Element::Element(const unsigned int idParam, const unsigned int noiseParam): id(idParam), noise(noiseParam) 
{
}

// Return true if this element has less noisy then the other. False otherwise.
const bool Element::operator<(const Element& otherElement) const
{
  return noise < otherElement.noise;
}

// Return this element's ID
const unsigned int Element::getId() const
{
  return id;
}

// Return this element's noise
const unsigned int Element::getNoise() const
{
  return noise;
}

// Increase this element's noise by n
void Element::addNoise(const unsigned int n)
{
  noise += n;
}

// Return true if this element has smaller ID then the other. False otherwise.
const bool Element::smallerId(const Element& e1, const Element& e2)
{
  return e1.id < e2.id;
}

// Unites 2 id-sorted element's vector
vector<unsigned int> Element::idVectorUnion(const vector<Element>& v1,const vector<Element>& v2)
{
  vector<unsigned int> unionVector;	// Union resulted vector to be returned
  unionVector.reserve(v1.size() + v2.size());
  vector<Element>::const_iterator v1It = v1.begin();
  vector<Element>::const_iterator v2It = v2.begin();
  while (true)
    {
     // Case 1: v1 has smaller id
     // Note: why not use smallerId method?
      if (v1It->id < v2It->id)
	{
	  // start unionVector with v1
	  unionVector.push_back(v1It->id);
	  // is v1 finished?
	  if (++v1It == v1.end())
	    {
	      // so append each v2 element to the unionVector
	      for (; v2It != v2.end(); ++v2It)
		{
		  unionVector.push_back(v2It->id);
		}
	      return unionVector;
	    }
	}
      else
	{
	  // Case 2: v1 and v2's ids are the same
	  if (v1It->id == v2It->id)
	    {
	      // 1st elements are the same, v1 has only one element. So, just copy v2.
	      if (++v1It == v1.end())
		{
		  for (; v2It != v2.end(); ++v2It)
		    {
		      unionVector.push_back(v2It->id);
		    }
		  return unionVector;
		}

	      // NOTE: in the case the following if is true, we'll end up with a redundant element.
	      //	1 - ID's are the same.
	      //	2 - Append v2.
	      //	3 - V2 had only one element.
	      //	4 - append every v1 element (including the first one, which was already added in the second step)
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
	  // case 3: v2 has smaller id	
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
