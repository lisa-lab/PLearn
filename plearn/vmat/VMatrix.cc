// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2004 Rejean Ducharme
//

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org


 
/*
* $Id: VMatrix.cc,v 1.76 2004/11/26 14:48:49 tihocan Exp $
******************************************************* */

#include "VMatrix.h"
#include "DiskVMatrix.h"
#include "FileVMatrix.h"
#include "SubVMatrix.h"
#include "VMat_computeStats.h"


namespace PLearn {
using namespace std;

/** VMatrix **/


PLEARN_IMPLEMENT_ABSTRACT_OBJECT(VMatrix, "ONE LINE DESCR", "NO HELP");

VMatrix::VMatrix()
  : lockf_(0), length_(-1), width_(-1), mtime_(0), 
   inputsize_(-1), targetsize_(-1), weightsize_(-1),
   writable(false)
{}

VMatrix::VMatrix(int the_length, int the_width)
  : lockf_(0), length_(the_length), width_(the_width), mtime_(0), 
   inputsize_(-1), targetsize_(-1), weightsize_(-1),
   writable(false),
   map_sr(TVec<map<string,real> >(the_width)),
   map_rs(TVec<map<real,string> >(the_width)),
   fieldstats(0)
{}

void VMatrix::declareOptions(OptionList & ol)
{
  declareOption(ol, "writable", &VMatrix::writable, OptionBase::buildoption, "Are write operations permitted?");
  declareOption(ol, "length", &VMatrix::length_, OptionBase::buildoption, 
                "length of the matrix (number of rows)");
  declareOption(ol, "width", &VMatrix::width_, OptionBase::buildoption, 
                "width of the matrix (number of columns; -1 indicates this varies from sample to sample...)");
  declareOption(ol, "inputsize", &VMatrix::inputsize_, OptionBase::buildoption, 
                "size of input part (-1 if variable or unspecified, 0 if no input)");
  declareOption(ol, "targetsize", &VMatrix::targetsize_, OptionBase::buildoption, 
                "size of target part (-1 if variable or unspecified, 0 if no target)");
  declareOption(ol, "weightsize", &VMatrix::weightsize_, OptionBase::buildoption, 
                "size of weights (-1 if unspecified, 0 if no weight, 1 for sample weight, >1 currently not supported (include it is recommended to include additional info in target. weight is really reserved for a per sample weight)).");
  declareOption(ol, "metadatadir", &VMatrix::metadatadir, OptionBase::buildoption, 
                "A directory in which to store meta-information for this matrix \n"
                "You don't always have to give this explicitly. For ex. if your \n"
                "vmat is the outer VMatrix in a .vmat file, the metadatadir will \n"
                "automatically be set to name_of_vmat_file.metadata/ \n"
                "And if it is the source inside another VMatrix that sets its \n"
                "metadatadir, it will often be set from that surrounding vmat's metadata \n");
  inherited::declareOptions(ol);
}

void VMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(get_row, copies);
  deepCopyField(dotrow_1, copies);
  deepCopyField(dotrow_2, copies);
  deepCopyField(field_stats, copies);
  deepCopyField(map_sr, copies);
  deepCopyField(map_rs, copies);
  deepCopyField(fieldinfos, copies);
  deepCopyField(fieldstats, copies);
}

Array<VMField>& VMatrix::getFieldInfos() const
{
  if(fieldinfos.size()==0 && hasMetaDataDir())
    {
      string fname =  append_slash(getMetaDataDir()) + "fieldnames";
      if(isfile(fname)) // file exists
        loadFieldInfos();
    }

  int ninfos = fieldinfos.size();
  int w = width();
  if(ninfos!=w && w > 0)
    {
      fieldinfos.resize(w);
      for(int j=ninfos; j<w; j++)
        fieldinfos[j] = VMField(tostring(j));
    }

  return fieldinfos;
}

void VMatrix::setFieldInfos(const Array<VMField>& finfo)
{
  fieldinfos=finfo;
}

bool VMatrix::hasFieldInfos() const
{
  return fieldinfos.size()>0;
}

void VMatrix::unduplicateFieldNames()
{
  map<string,vector<int> > mp;
  for(int i=0;i<width();i++)
    mp[getFieldInfos(i).name].push_back(i);
  map<string,vector<int> >::iterator it;
  for(it=mp.begin();it!=mp.end();++it)
    if(it->second.size()!=1)
      {
        vector<int> v=it->second;
        for(unsigned int j=0;j<v.size();j++)
          fieldinfos[v[j]].name+="."+tostring(j);
      }
}

TVec<string> VMatrix::fieldNames() const
{
  int d = width();
  TVec<string> names(d);
  for(int i=0; i<d; i++)
    names[i] = fieldName(i);
  return names;
}

int VMatrix::fieldIndex(const string& fieldname) const
{
  Array<VMField>& infos = getFieldInfos(); 
  for(int i=0; i<width(); i++)
    if(infos[i].name==fieldname)
      return i;
  return -1;
}

int VMatrix::getFieldIndex(const string& fieldname_or_num) const
{
  int i = fieldIndex(fieldname_or_num);
  if(i==-1)
    i = toint(fieldname_or_num);
  if (i < 0 || i >= width())
    PLERROR("In VMatrix::getFieldIndex - Asked for an unvalid column number");
  return i;
}

void VMatrix::build_()
{
  if(metadatadir!="")
    setMetaDataDir(metadatadir); // make sure we perform all necessary operations 
}

void VMatrix::build()
{
  inherited::build();
  build_();
}

void VMatrix::printFieldInfo(ostream& out, int fieldnum) const
{
    VMField fi = getFieldInfos(fieldnum);
    StatsCollector& s = getStats(fieldnum);

    out << "Field #" << fieldnum << ":  ";
    out << fi.name << "\t type: ";
    switch(fi.fieldtype)
      {
      case VMField::UnknownType:
        out << "UnknownType\n";
        break;
      case VMField::Continuous:
        out << "Continuous\n";
        break;
      case VMField::DiscrGeneral:
        out << "DiscrGeneral\n";
        break;
      case VMField::DiscrMonotonic:
        out << "DiscrMonotonic\n";
        break;
      case VMField::DiscrFloat:
        out << "DiscrFloat\n";
        break;
      case VMField::Date:
        out << "Date\n";
        break;
      default:
        PLERROR("Can't write name of type");
      }  

    out.precision(12);
    out << "nmissing: " << s.nmissing() << '\n';
    out << "nnonmissing: " << s.nnonmissing() << '\n';
    out << "sum: " << s.sum() << '\n';
    out << "mean: " << s.mean() << '\n';
    out << "stddev: " << s.stddev() << '\n';
    out << "min: " << s.min() << '\n';
    out << "max: " << s.max() << '\n';

    /*
    if(!s.counts.empty())
      {
        out << "\nCOUNTS: \n";
        map<real,StatsCollectorCounts>::const_iterator it = s.counts.begin();
        map<real,StatsCollectorCounts>::const_iterator countsend = s.counts.end();
        while(it!=countsend)
          {
            real val = it->first;
            const StatsCollectorCounts& co = it->second;
            out << "  " << val;
            string s = getValString(fieldnum, val);          
            if(s!="")
              out << " " << s;
            out << " \t: n=" << co.n << "\t nbelow=" << co.nbelow << "\t sumbelow=" << co.sum << endl; 
            ++it;
        }
      }
    */
    out << endl << endl;
}

void VMatrix::printFieldInfo(ostream& out, const string& fieldname_or_num) const
{
  printFieldInfo(out, getFieldIndex(fieldname_or_num));  
}

void VMatrix::printFields(ostream& out) const
{ 
  for(int j=0; j<width(); j++)
    {
    printFieldInfo(out,j);
    out << "-----------------------------------------------------" << endl;  
    }
}

void VMatrix::getExample(int i, Vec& input, Vec& target, real& weight) 
{
  if(inputsize_<0)
    PLERROR("In VMatrix::getExample, inputsize_ not defined for this vmat");
  input.resize(inputsize_);             
  getSubRow(i,0,input);
  if(targetsize_<0)
    PLERROR("In VMatrix::getExample, targetsize_ not defined for this vmat");
  target.resize(targetsize_);
  if (targetsize_ > 0) {
    getSubRow(i,inputsize_,target);
  }

  if(weightsize_==0)
    weight = 1;
  else if(weightsize_<0)
    PLERROR("In VMatrix::getExample, weightsize_ not defined for this vmat");
  else if(weightsize_>1)
    PLERROR("In VMatrix::getExample, weightsize_ >1 not supported by this call");
  else
    weight = get(i,inputsize_+targetsize_);
}


void VMatrix::computeStats()
{
  fieldstats = Array<VMFieldStat>(width());
  Vec row(width());
  for(int i=0; i<length(); i++)
    {
      getRow(i,row);
      for(int j=0; j<width(); j++)
        fieldstats[j].update(row[j]);
    }
}

void VMatrix::loadStats(const string& filename)
{
  ifstream in(filename.c_str());
  if(!in)
    PLERROR("In VMatrix::loadStats Couldn't open file %s for reading",filename.c_str());
  int nfields;
  in >> nfields;
  if(nfields!=width())
    PLWARNING("In VMatrix::loadStats nfields differes from VMat width");

  fieldstats.resize(nfields);
  for(int j=0; j<fieldstats.size(); j++)
    fieldstats[j].read(in);
}

void VMatrix::saveStats(const string& filename) const
{
  ofstream out(filename.c_str());
  if(!out)
    PLERROR("In VMatrix::saveStats Couldn't open file %s for writing",filename.c_str());
  out << fieldstats.size() << endl;
  for(int j=0; j<fieldstats.size(); j++)
  {
    fieldstats[j].write(out);
    out << endl;
  }
}


string VMatrix::fieldheader(int elementcharwidth)
{
  // Implementation not done yet

  return "VMatrix::fieldheader NOT YET IMPLEMENTED";
}


void VMatrix::declareField(int fieldindex, const string& fieldname, VMField::FieldType fieldtype)
{ getFieldInfos(fieldindex) = VMField(fieldname,fieldtype); }

void VMatrix::declareFieldNames(TVec<string> fnames)
{
  if(fnames.length()!=width())
    PLERROR("In VMatrix::declareFieldNames length of fnames differs from width() of VMatrix");
  for(int i=0; i<fnames.length(); i++)
    declareField(i,fnames[i]);
}

void VMatrix::saveFieldInfos() const
{
  // check if we need to save the fieldinfos
  if(fieldinfos.size()==0)
    return;
  Array<VMField> current_fieldinfos = getSavedFieldInfos();
  if (current_fieldinfos==fieldinfos)
    return;

  string filename = append_slash(getMetaDataDir()) + "fieldnames";
  ofstream out(filename.c_str());
  if(!out)
    PLERROR("In VMatrix::saveFieldInfos Couldn't open file %s for writing",filename.c_str());    
  for(int i= 0; i < fieldinfos.length(); ++i)
    out << fieldinfos[i].name << '\t' << fieldinfos[i].fieldtype << endl;
}

void VMatrix::loadFieldInfos() const
{
  Array<VMField> current_fieldinfos = getSavedFieldInfos();
  fieldinfos = current_fieldinfos;
}

Array<VMField> VMatrix::getSavedFieldInfos() const
{
  string filename = append_slash(getMetaDataDir()) + "fieldnames";
  if (!isfile(filename)) // no current fieldinfos saved
  {
    Array<VMField> no_fieldinfos(0);
    return no_fieldinfos;
  }
  ifstream in(filename.c_str());
  if(!in)
    PLERROR("In VMatrix::getSavedFieldInfos Couldn't open file %s for reading",filename.c_str());    

  int w = width();
  Array<VMField> current_fieldinfos(w);
  for(int i=0; i<w; ++i)
  {
    vector<string> v(split(pgetline(in)));
    switch(v.size())
    {
      case 1: current_fieldinfos[i] = VMField(v[0]); break;
      case 2: current_fieldinfos[i] = VMField(v[0], VMField::FieldType(toint(v[1]))); break;
      default: PLERROR("In VMatrix::getSavedFieldInfos Format not recognized in file %s.\n"
                       "Each line should be '<name> {<type>}'.", filename.c_str());
    }
  }
  return current_fieldinfos;
}

// comments: see .h
string VMatrix::resolveFieldInfoLink(string target, string source)
{
  string contents = removeblanks(loadFileAsString(source));
  if(contents==source)
    return "ERROR";
  if(isdir(contents))
  {
    //just in case it lacks a slash..
    contents+=slash;
    if(isfile(contents+target+".lnk"))
      return resolveFieldInfoLink(target,contents+target+".lnk");
    else if(isfile(contents+target))
      return contents+target;
    else if(isfile(contents+slash+"__default.lnk"))
      return resolveFieldInfoLink(target,contents+slash+"__default.lnk");
    // assume target is there, but file is empty thus inexistant
    else return contents+target;
  }
  else if(contents.substr(contents.size()-4,4)==".lnk")
    return resolveFieldInfoLink(target,contents);
  else return contents;
}

void VMatrix::setSFIFFilename(int col, string ext, string filepath)
{
  setSFIFFilename(fieldName(col),ext,filepath);
}


void VMatrix::setSFIFFilename(string fieldname, string ext, string filepath)
{
  string target = makeFileNameValid(fieldname+ext);
  string normalfname = getMetaDataDir()+"FieldInfo"+slash+target;
  rm(normalfname+".lnk");
  if(filepath==normalfname || filepath=="")
  {
    rm(normalfname+".lnk");
    return;
  }

  force_mkdir_for_file(normalfname);
  ofstream o((normalfname+".lnk").c_str());
  o<<filepath<<endl;
}

string VMatrix::getSFIFFilename(int col, string ext)
{
  return getSFIFFilename(fieldName(col),ext);
}

string VMatrix::getSFIFFilename(string fieldname, string ext)
{
  string target = makeFileNameValid(fieldname+ext);
  string normalfname = getMetaDataDir()+"FieldInfo"+slash+target;
  string defaultlinkfname = getMetaDataDir()+"FieldInfo"+slash+"__default.lnk";
  if(isfile(normalfname))
    return normalfname;
  else if(isfile(normalfname+".lnk"))
    return resolveFieldInfoLink(target, normalfname+".lnk");
  else if(isfile(defaultlinkfname))
    return resolveFieldInfoLink(target, defaultlinkfname);
  // assume target is here, but file is empty thus inexistant
  else return normalfname;
}

bool VMatrix::isSFIFDirect(int col, string ext)
{
  return isSFIFDirect(fieldName(col), ext);
}

bool VMatrix::isSFIFDirect(string fieldname, string ext)
{
  string target = makeFileNameValid(fieldname+ext);
  string normalfname = getMetaDataDir()+"FieldInfo"+slash+target;
  return getSFIFFilename(fieldname,ext) == normalfname;
}

//! adds a string<->real mapping
void VMatrix::addStringMapping(int col, string str, real val)
{
  init_map_sr();
  map_sr[col][str]=val;
  map_rs[col][val]=str;
}

real VMatrix::addStringMapping(int col, string str)
{
  init_map_sr();
  map<string,real>& m = map_sr[col];
  map<string,real>::iterator it = m.find(str);

  real val = 0;
  if(it != m.end()) // str was found in map
    val = it->second;
  else // str not found in map: add a new mapping
    {
      val = - m.size() - 100;
      addStringMapping(col, str, val);
    }
  return val;
}

void VMatrix::removeAllStringMappings()
{
  init_map_sr();
  for(int i=0;i<width();i++)
    {
      map_sr[i].clear();
      map_rs[i].clear();
    }
}

void VMatrix::removeColumnStringMappings(int c)
{
  init_map_sr();
  map_sr[c].clear();
  map_rs[c].clear();
}

///////////////////////////
// saveAllStringMappings //
///////////////////////////
void VMatrix::saveAllStringMappings()
{
  string fname;
  for(int i=0;i<width();i++)
  {
    fname = getSFIFFilename(i,".smap");
    saveStringMappings(i,fname);
  }
}

void VMatrix::saveStringMappings(int col,string fname)
{
  map<string, real> the_map = getStringToRealMapping(col);
  if(the_map.size()==0)
  {
    rm(fname);
    return;
  }
  force_mkdir_for_file(fname);
  POFStream o(fname.c_str());
  // ofstream o(fname.c_str());
  if(!o)
    PLERROR( "File %s can't be opened",fname.c_str());
  for(map<string,real>::iterator it = the_map.begin();it!=the_map.end();++it)
    o << it->first << it->second << endl;
}

//! removes a string mapping
void VMatrix::removeStringMapping(int col, string str)
{
  init_map_sr();
  map<string,real>::iterator sriterator;
// check if the mapping ractually exists
  if((sriterator = map_sr[col].find(str)) == map_sr[col].end())
    return;
  real val = map_sr[col][str];
  map_sr[col].erase(sriterator);
  map_rs[col].erase(map_rs[col].find(val));
}

//! overwrite the string<->real mapping with this one (and build the reverse mapping)
void VMatrix::setStringMapping(int col, const map<string,real> & zemap)
{
  init_map_sr();
  map_sr[col]=zemap;
  map_rs[col].clear();
  for(map<string,real>::iterator it = map_sr[col].begin();it!=map_sr[col].end();++it)
    map_rs[col][it->second]=it->first;
}

//! deletes all string mapping for column i
void VMatrix::deleteStringMapping(int col)
{
  init_map_sr();
  if(col>=map_sr.size() ||
     col>=map_rs.size())
    PLERROR("deleteStringMapping : out of bounds for col=%i in string mapping array (size=%i).\n Current VMatrix\nclass"\
            "is '%s' (or maybe derivated class?). be sure to set\n map_sr(rs) to appropriate sizes as soon as you know the width of the matrix\n"\
            "(in constructor or elsewhere)",col,map_sr.size(),classname().c_str());
  map_sr[col].clear();
  map_rs[col].clear();
}

//////////////////
// getValString //
//////////////////
string VMatrix::getValString(int col, real val) const
{ 
  if(is_missing(val))
    return "";
  init_map_sr();
  if(map_rs[col].find(val)==map_rs[col].end())
    return "";
  else return map_rs[col][val];
}

//////////////////
// getStringVal //
//////////////////
real VMatrix::getStringVal(int col,const string & str) const
{
  if(map_sr.length()==0 || map_sr[col].find(str)==map_sr[col].end())
    return MISSING_VALUE;
  else return map_sr[col][str];
}

///////////////
// getString //
///////////////
string VMatrix::getString(int row,int col) const
{
  static string str;
  real val = get(row,col);
  str = getValString(col, val);
  if (str == "")
    // There is no string mapping associated to this value.
    return tostring(val);
  else
    return str;
}

////////////////////////////
// getStringToRealMapping //
////////////////////////////
const map<string,real>& VMatrix::getStringToRealMapping(int col) const {
  init_map_sr();
  return map_sr[col];
}

////////////////////////////
// getRealToStringMapping //
////////////////////////////
const map<real,string>& VMatrix::getRealToStringMapping(int col) const {
  init_map_sr();
  return map_rs[col];
}

////////////////////
// setMetaDataDir //
////////////////////
void VMatrix::setMetaDataDir(const string& the_metadatadir) 
{ 
  if(the_metadatadir=="")
    PLERROR("Called setMetaDataDir with an empty string");
  metadatadir = the_metadatadir; 
  if(!force_mkdir(metadatadir))
    PLERROR("In VMatrix::setMetadataDir could not create directory %s",metadatadir.c_str());
  metadatadir = abspath(metadatadir);
}

//////////////////
// getDimension //
//////////////////
int VMatrix::getDimension(int row, int col) const
{
  //! Return the dimension of the column col, -1 if continuous 
  return -1;
}  

///////////////////
// copySizesFrom //
///////////////////
void VMatrix::copySizesFrom(const VMat& m) {
  defineSizes(m->inputsize(), m->targetsize(), m->weightsize());
}

/////////////////////
// setMetaInfoFrom //
/////////////////////
void VMatrix::setMetaInfoFrom(const VMat& vm)
{
  setMtime(max(getMtime(),vm->getMtime()));

  // copy length and width from vm if not set
  if(length_<0)
    length_ = vm->length();
  if(width_<0)
    width_ = vm->width();

  // copy sizes from vm if not set
  if(inputsize_<0)
    inputsize_ = vm->inputsize();
  if(targetsize_<0)
    targetsize_ = vm->targetsize();
  if(weightsize_<0)
    weightsize_ = vm->weightsize();

  // copy fieldnames from vm if not set and they look good
  if(!hasFieldInfos() && (width() == vm->width()) && vm->hasFieldInfos() )
    setFieldInfos(vm->getFieldInfos());

  // Copy String/Real mappings if not set.
  if (map_rs.length() == 0) {
    map_rs.resize(width_);
    for (int j = 0; j < width_; j++) {
      if (j < vm->width()) {
        map_rs[j] = vm->getRealToStringMapping(j);
      } else {
        // Empty map.
        map_rs[j] = map<real,string>();
      }
    }
  }
  if (map_sr.length() == 0) {
    map_sr.resize(width_);
    for (int j = 0; j < width_; j++) {
      if (j < vm->width()) {
        map_sr[j] = vm->getStringToRealMapping(j);
      } else {
        // Empty map.
        map_sr[j] = map<string,real>();
      }
    }
  }
}
 
////////////////////
// looksTheSameAs //
////////////////////
bool VMatrix::looksTheSameAs(const VMat& m) {
  return !(
         this->width()      != m->width()
      || this->length()     != m->length()
      || this->inputsize()  != m->inputsize()
      || this->weightsize() != m->weightsize()
      || this->targetsize() != m->targetsize() );
}

string getHost()
{
  return "TODO";
}

int getPid()
{
  return -999;
}

string getUser()
{
  return "TODO";
}

void VMatrix::lockMetaDataDir() const
{
  if(!hasMetaDataDir())
    PLERROR("In VMatrix::lockMetaDataDir(): metadatadir was not set");
  if(lockf_!=0) // already locked by this object!
    PLERROR("VMatrix::lockMetaDataDir() called while already locked by this object.");
  if(!pathexists(metadatadir))
    force_mkdir(metadatadir);
  string lockfile = append_slash(metadatadir)+".lock";  
  lockf_ = fopen(lockfile.c_str(),"w");  
  if(lockf_==0) //! locked by somebody else
    {
      string bywho;
      try{ bywho = loadFileAsString(lockfile); }
      catch(...) { bywho = "UNKNOWN (could not read .lock file)"; }

      cerr << "! Waiting for .lock on directory " << metadatadir 
           << " created by " << bywho << endl;
    }
  do
    { 
      // try again after a second
      sleep(1);
      lockf_ = fopen(lockfile.c_str(),"w");        
    }   while(lockf_==0);
  
  fprintf(lockf_, "host %s, pid %d, user %s", getHost().c_str(), getPid(), getUser().c_str());
  fflush(lockf_);   // Don't close it: to keep the lock!

}

void VMatrix::unlockMetaDataDir() const
{
  if(lockf_==0)
    PLERROR("In VMatrix::unlockMetaDataDir() was called while no lock is held by this object");
  fclose(lockf_);
  string lockfile = append_slash(metadatadir)+".lock";  
  rm(lockfile);
  lockf_ = 0;
}

string VMatrix::getMetaDataDir() const
{ 
  //  if(!hasMetaDataDir())
  //  PLERROR("In VMatrix::getMetaDataDir(): metadatadir was not set"); 
  return metadatadir; 
}

///////////////////////////
// loadAllStringMappings //
///////////////////////////
void VMatrix::loadAllStringMappings()
{
  // if this is a StrTableVMatrix, smap are already created
  if(classname()=="StrTableVMatrix")
    return;
  for(int i=0;i<width();i++)
    loadStringMapping(i);
}

///////////////////////
// loadStringMapping //
///////////////////////
void VMatrix::loadStringMapping(int col)
// loads the appropriate string map file for column 'col'
{
  if(!hasMetaDataDir())
    return;
  string fname = getSFIFFilename(col,".smap");
  init_map_sr();
  force_mkdir(getMetaDataDir()+"FieldInfo"+slash);

  if(!isfile(fname))
  {
//     ofstream o(fname.c_str());
//     if(o.bad())
//       PLERROR( string("\nEmpty new file "+fname+" could not be created.\n (This is ony done to check consistency of path. File is deleted afterward.)").c_str());
//     rm(fname);
    return;
  }
  
  deleteStringMapping(col);

  // smap file exists, open it
  PIFStream f(fname);
  if(!f)
    PLERROR( string("File "+fname+" cannot be opened.").c_str());

/*  string pref;
  f>>pref;
  if(string(pref)!="#SMAP")
    PLERROR( string("File "+fname+" is not a valid String mapping file.\nShould start with #SMAP on first line (this is to prevent inopportunely overwritting another type of file)").c_str());
*/
  while(f)
  {
    string s;
    real val;
    f >> s >> val;
    if(f) 
    {
      map_sr[col][s]= val;
      map_rs[col][val]=s;
    }
  }
}

////////////////////////////
// copyStringMappingsFrom //
////////////////////////////
void VMatrix::copyStringMappingsFrom(VMat source) {
  if (width_ != source->width()) {
    PLERROR("In VMatrix::copyStringMappingsFrom - The source VMatrix doesn't have the same width");
  }
  map_rs.resize(width_);
  map_sr.resize(width_);
  for (int i = 0; i < width_; i++) {
    setStringMapping(i, source->getStringToRealMapping(i));
  }
}

//! returns the unconditonal statistics for the given field
TVec<StatsCollector> VMatrix::getStats() const
{
  if(!field_stats)
  {
    string statsfile = getMetaDataDir() + slash+"stats.psave";
    if (isfile(statsfile) && getMtime()<mtime(statsfile))
    {
      if(getMtime()==0)
        PLWARNING("Warning: using a saved stat file (%s) but mtime is 0.\n(cannot be sure file is up to date)",statsfile.c_str());
      PLearn::load(statsfile, field_stats);
    }
    else
    {
      VMat vm = const_cast<VMatrix*>(this);
      field_stats = PLearn::computeStats(vm, 2000);
      PLearn::save(statsfile, field_stats);
    }
  }
  return field_stats;
}

TVec<RealMapping> VMatrix::getRanges()
{
  TVec<RealMapping> ranges;
  string rangefile = getMetaDataDir() + slash+"ranges.psave";
  if(isfile(rangefile))
    PLearn::load(rangefile, ranges);
  else
  {
    ranges = computeRanges(getStats(),std::max(10,length()/200),std::max(10,length()/100) );
    PLearn::save(rangefile, ranges);
  }
  return ranges;
}

// Eventually to be changed to pure virtual, once get has been implemented in all subclasses
// calls to sample can then be replaced by getRow everywhere
real VMatrix::get(int i, int j) const
{
  PLERROR("get(i,j) method not implemented for this VMat (name=%s), please implement.",classname().c_str());
  return 0.0;
}

void VMatrix::put(int i, int j, real value)
{
  PLERROR("put(i,j,value) method not implemented for this VMat, please implement.");
}

void VMatrix::getColumn(int j, Vec v) const
{
#ifdef BOUNDCHECK
  if(v.length() != length())
    PLERROR("In VMatrix::getColumn v must have the same length as the VMatrix");
#endif
  for(int i=0; i<v.length(); i++)
    v[i] = get(i,j);
}

///////////////
// getSubRow //
///////////////
void VMatrix::getSubRow(int i, int j, Vec v) const
{
  for(int k=0; k<v.length(); k++)
    v[k] = get(i,j+k);
}

void VMatrix::putSubRow(int i, int j, Vec v)
{
  for(int k=0; k<v.length(); k++)
    put(i, j+k, v[k]);
}

////////////
// getRow //
////////////
void VMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if(v.length() != width())
    PLERROR("In VMatrix::getRow(i,v) length of v and width of VMatrix differ");
#endif
  getSubRow(i,0,v);
}

void VMatrix::putRow(int i, Vec v)
{
#ifdef BOUNDCHECK
  if(v.length() != width())
    PLERROR("In VMatrix::putRow(i,v) length of v and width of VMatrix differ");
#endif
  putSubRow(i,0,v);
}

void VMatrix::fill(real value)
{
  Vec v(width(), value);
  for (int i=0; i<length(); i++) putRow(i,v);
}

void VMatrix::appendRow(Vec v)
{
  PLERROR("This method (appendRow) not implemented by VMatrix subclass!");
}

void VMatrix::flush()
{}

void VMatrix::putOrAppendRow(int i, Vec v)
{
  if(i==length())
    appendRow(v);
  else if(i<length())
    putRow(i,v);
  else
    PLERROR("In putOrAppendRow, index %d out of range",i);
}

void VMatrix::forcePutRow(int i, Vec v)
{
  if(i<length())
    putRow(i,v);
  else 
    {
      Vec emptyrow(width());
      emptyrow.clear();
      while(length()<i)
        appendRow(emptyrow);
      appendRow(v);
    }
}

void VMatrix::getMat(int i, int j, Mat m) const
{
#ifdef BOUNDCHECK
  if(i<0 || j<0 || i+m.length()>length() || j+m.width()>width())
    PLERROR("In VMatrix::getMat(i,j,m) OUT OF BOUNDS");
#endif
  for(int ii=0; ii<m.length(); ii++)
    {
      Vec v = m(ii);
      getSubRow(i+ii, j, v);
    }
}

void VMatrix::putMat(int i, int j, Mat m)
{
#ifdef BOUNDCHECK
  if(i<0 || j<0 || i+m.length()>length() || j+m.width()>width())
    PLERROR("In VMatrix::putMat(i,j,m) OUT OF BOUNDS");
#endif
  for(int ii=0; ii<m.length(); ii++)
    {
      Vec v = m(ii);
      putSubRow(i+ii, j, v);
    }
}

void VMatrix::compacify() {}


Mat VMatrix::toMat() const
{
  Mat m(length(),width());
  getMat(0,0,m);
  return m;
}

VMat VMatrix::subMat(int i, int j, int l, int w)
{ return new SubVMatrix(this,i,j,l,w); }

/////////
// dot //
/////////
real VMatrix::dot(int i1, int i2, int inputsize) const
{ 
  dotrow_1.resize(inputsize);
  dotrow_2.resize(inputsize);
  getSubRow(i1, 0, dotrow_1);
  getSubRow(i2, 0, dotrow_2);
  return PLearn::dot(dotrow_1, dotrow_2);
}

real VMatrix::dot(int i, const Vec& v) const
{
  dotrow_1.resize(v.length());
  getSubRow(i, 0, dotrow_1);
  return PLearn::dot(dotrow_1, v);
}


//////////
// find //
//////////
bool VMatrix::find(const Vec& input, real tolerance, int* i) const {
  get_row.resize(inputsize());
#ifdef BOUNDCHECK
  if (input.length() != inputsize())
    PLERROR("In VMatrix::find - The given vector must be the same size as inputsize");
#endif
  for (int j = 0; j < length(); j++) {
    getSubRow(j, 0, get_row);
    if (powdistance(input, get_row, 2.0) < tolerance) {
      if (i)
        *i = j;
      return true;
    }
  }
  if (i)
    *i = -1;
  return false;
}

void VMatrix::print(ostream& out) const
{
  Vec v(width());
  for(int i=0; i<length(); i++)
    {
      getRow(i,v);
      out << v << endl;
    }
}

/*
void VMatrix::oldwrite(ostream& out) const
{
  writeHeader(out,"VMatrix");
  writeField(out,"length_", length_);
  writeField(out,"width_", width_);
  //writeField(out,"fieldinfos", fieldinfos);
  //writeField(out,"fieldstats", fieldstats);
  writeFooter(out,"VMatrix");
}

void VMatrix::oldread(istream& in)
{
  readHeader(in,"VMatrix");
  readField(in,"length_", length_);
  readField(in,"width_", width_);
  //readField(in,"fieldinfos", fieldinfos);
  //readField(in,"fieldstats", fieldstats);
  readFooter(in,"VMatrix");
}
*/


VMatrix::~VMatrix()
{}

void VMatrix::save(const string& filename) const
{ savePMAT(filename); }

void VMatrix::savePMAT(const string& pmatfile) const
{
  if (width() == -1)
    PLERROR("In VMat::save Saving in a pmat file is only possible for constant width Distributions (where width()!=-1)");

  int nsamples = length();

  FileVMatrix m(pmatfile,nsamples,width());
  m.setFieldInfos(getFieldInfos());
  Vec tmpvec(width());

  ProgressBar pb(cout, "Saving to pmat", nsamples);

  for(int i=0; i<nsamples; i++)
  {
    getRow(i,tmpvec);
    m.putRow(i,tmpvec);
    pb(i);
  }

  //save field names if necessary
  if (fieldinfos.size() > 0) m.saveFieldInfos();      
}

void VMatrix::saveDMAT(const string& dmatdir) const
{
  force_rmdir(dmatdir);  
  DiskVMatrix vm(dmatdir,width());
  vm.setFieldInfos(getFieldInfos());
  Vec v(width());

  ProgressBar pb(cout, "Saving to dmat", length());

  for(int i=0;i<length();i++)
  {
    getRow(i,v);
    vm.appendRow(v);
    pb(i);
    //cerr<<i<<" "<<flush;
  }
}

//////////////
// saveAMAT //
//////////////
void VMatrix::saveAMAT(const string& amatfile, bool verbose, bool no_header) const
{
  int l = length();
  int w = width();

  ofstream out(amatfile.c_str());
  if (!out)
   PLERROR("In saveAscii could not open file %s for writing",amatfile.c_str());

  if (!no_header) {
    out << "#size: "<< l << ' ' << w << endl;
  }
  out.precision(15);
  if(w>0 && !no_header)
    {
      out << "#: ";
      for(int k=0; k<w; k++)
        //there must not be any space in a field name...
        out << space_to_underscore(fieldName(k)) << ' ';
      out << "\n";
    }

  Vec v(w);

  ProgressBar* pb = 0;
  if (verbose)
    pb = new ProgressBar(cout, "Saving to amat", length());

  for(int i=0;i<l;i++)
    {
      getRow(i,v);
      for(int j=0; j<w; j++)
        out << v[j] << ' ';
      out << "\n";
      if (verbose)
         pb->update(i);
    }
  if (verbose)
    delete pb;
}

// result += transpose(X).Y
// Where X = this->subMatColumns(X_startcol,X_ncols)
// and   Y =  this->subMatColumns(Y_startcol,Y_ncols);
void VMatrix::accumulateXtY(int X_startcol, int X_ncols, int Y_startcol, int Y_ncols, 
                            Mat& result, int startrow, int nrows, int ignore_this_row) const
{
  int endrow = (nrows>0) ?startrow+nrows :length_;
  Vec x(X_ncols);
  Vec y(Y_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,X_startcol,x);
      getSubRow(i,Y_startcol,y);
      externalProductAcc(result, x,y);
    }
}

// result += transpose(X).Y
// Where X = this->subMatColumns(X_startcol,X_ncols)
void VMatrix::accumulateXtX(int X_startcol, int X_ncols, 
                            Mat& result, int startrow, int nrows, int ignore_this_row) const
{
  Vec x(X_ncols);
  int endrow = (nrows>0) ?startrow+nrows :length_;
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,X_startcol,x);
      externalProductAcc(result, x,x);
    }
}



} // end of namespace PLearn
