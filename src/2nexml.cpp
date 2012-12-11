/*******************************************************************************
 * This file contains is taken from NCL's normalizer example
 */
#include <cstdio>
#include <cassert>
#include "ncl/ncl.h"
#include "ncl/nxsblock.h"
#include "ncl/nxspublicblocks.h"
#include "ncl/nxsmultiformat.h"
#if defined HAVE_CONFIG
#	include "config.h"
#endif
#if ! defined(NCL_NAME_AND_VERSION)
#	define NCL_NAME_AND_VERSION "NCL unknown version"
#endif
using namespace std;

#define TO_NEXML_CONVERTER 1

void writeAsNexml(PublicNexusReader & nexusReader, ostream & os);


class NexmlIDStrorer;

typedef std::pair<const NxsDiscreteDatatypeMapper *, std::vector<std::string> > MapperStateLabelVec;
template <typename T>
std::string getOrGenId(std::pair<T, unsigned> & p, std::map<T, std::string> & toId, std::map<std::string, T> & toObj, const std::string & pref);

typedef std::pair<const NxsTaxaBlock *, unsigned> TaxaBlockPtrIndPair;
typedef std::pair<const NxsCharactersBlock *, unsigned> CharBlockPtrIndPair;
typedef std::pair<const NxsTreesBlock *, unsigned> TreesBlockPtrIndPair;
typedef std::pair<MapperStateLabelVec, unsigned> MapperStateLabelVecIndPair;

void writeOTUS(ostream & os, const NxsTaxaBlock *, const std::vector<const NxsAssumptionsBlock *> & assumps, NexmlIDStrorer &, unsigned );
void writeCharacters(ostream & os, const NxsCharactersBlock *, const std::vector<const NxsAssumptionsBlock *> & assumps, NexmlIDStrorer &, unsigned);
void writeTrees(ostream & os, const NxsTreesBlock *, const std::vector<const NxsAssumptionsBlock *> & assumps, NexmlIDStrorer &, unsigned);

typedef std::pair<std::string, std::string> AttributeData;
typedef std::vector<AttributeData> AttributeDataVec;

void writeAttribute(std::ostream & out, const AttributeData & aIt);




std::string generateID(std::string prefix, unsigned n)
	{
	char tmp[81];
	sprintf(tmp, "%u", n);
	prefix.append(tmp);
	return prefix;
	}


template <typename T>
std::string getOrGenId(std::pair<T, unsigned> & p, std::map<T, std::string> & toId, std::map<std::string, T> & toObj, const std::string & pref)
{
	typedef typename std::map<T, std::string>::const_iterator ToIDIterator;
	T & t = p.first;
	ToIDIterator f = toId.find(t);
	if (f == toId.end())
		{
		unsigned index = p.second;
		if (index == UINT_MAX)
			{
			cerr << "could not find ID for NEXUS object";
			exit(3);
			}
		std::string identifier;
		do
			{
			identifier = generateID(pref, index++);
			}
		while (toObj.find(identifier) != toObj.end());
		toId[t] = identifier;
		toObj[identifier] = t;
		return identifier;
		}
	return f->second;
}


class NexmlIDStrorer
	{
	public:

		std::string getID(TaxaBlockPtrIndPair taxa)
			{
			const std::string pref("t");
			return getOrGenId<const NxsTaxaBlock *>(taxa, taxaBToID, idToTaxaB, pref);
			}
		std::string getID(CharBlockPtrIndPair chars)
			{
			const std::string pref("c");
			return getOrGenId<const NxsCharactersBlock *>(chars, charsBToIDChar, idToCharsBChar, pref);
			}
		std::string getID(TreesBlockPtrIndPair trees)
			{
			const std::string pref("g");
			return getOrGenId<const NxsTreesBlock *>(trees, treesBToID, idToTreesB, pref);
			}
		std::string getID(TaxaBlockPtrIndPair taxa, unsigned taxonInd)
			{
			const std::string pref("t");
			std::string p =  getOrGenId<const NxsTaxaBlock *>(taxa, taxaBToID, idToTaxaB, pref);
			p.append(1, 'n');
			return generateID(p, taxonInd);
			}
		std::string getCharID(CharBlockPtrIndPair chars, unsigned charInd)
			{
			const std::string pref("c");
			std::string p =  getOrGenId<const NxsCharactersBlock *>(chars, charsBToIDChar, idToCharsBChar, pref);
			p.append(1, 'n');
			return generateID(p, charInd);
			}
		std::string getID(CharBlockPtrIndPair chars, unsigned charInd)
			{
			const std::string pref("r");
			std::string p =  getOrGenId<const NxsCharactersBlock *>(chars, charsBToIDRow, idToCharsBRow, pref);
			p.append(1, 'n');
			return generateID(p, charInd);
			}
		std::string getID(TreesBlockPtrIndPair trees, unsigned treeInd)
			{
			const std::string pref("g");
			std::string p =  getOrGenId<const NxsTreesBlock *>(trees, treesBToID, idToTreesB, pref);
			p.append(1, 'n');
			return generateID(p, treeInd);
			}
		std::string getID(MapperStateLabelVecIndPair m, unsigned sIndex)
			{
			const std::string pref("s");
			std::string p =  getOrGenId<MapperStateLabelVec>(m, mapperToID, idToMapper, pref);
			p.append(1, 'n');
			return generateID(p, sIndex);
			}


	protected:
		std::map<const NxsTaxaBlock *, std::string> taxaBToID;
		std::map<std::string, const NxsTaxaBlock *> idToTaxaB;
		std::map<const NxsCharactersBlock *, std::string> charsBToIDChar;
		std::map<std::string, const NxsCharactersBlock *> idToCharsBChar;
		std::map<const NxsCharactersBlock *, std::string> charsBToIDRow;
		std::map<std::string, const NxsCharactersBlock *> idToCharsBRow;
		std::map<const NxsTreesBlock *, std::string> treesBToID;
		std::map<std::string, const NxsTreesBlock *> idToTreesB;
		std::map<MapperStateLabelVec, std::string> mapperToID;
		std::map<std::string, MapperStateLabelVec> idToMapper;
	};





inline void writeAttribute(ostream & out, const AttributeData & aIt)
	{
	out << ' ' << aIt.first << '=';
	writeAttributeValue(out, aIt.second);
	}


class XMLElement
{
	public:

	XMLElement(const char *name, ostream &outstream, bool hasSubElements, const char *indentStr, const AttributeDataVec *ovec=NULL)
		:out(outstream),
		elementName(name)
		,containsElements(hasSubElements)
		,indentation(indentStr)
		{
		if (ovec)
			this->open(*ovec);
		}

	void open()
		{
		const std::vector<AttributeData> atts;
		this->open(atts);
		}

	void open(const std::vector<AttributeData> &atts)
		{
		out << indentation << "<" << this->elementName;
		std::vector<AttributeData>::const_iterator aIt = atts.begin();
		for (; aIt != atts.end(); ++aIt)
			{
			writeAttribute(out, *aIt);
			}
		if (containsElements)
			out << ">\n";
		else
			out << "/>\n";

		}
	virtual ~XMLElement()
		{
		if (containsElements)
			out << indentation << "</" << this->elementName << ">\n";
		}
	protected:
		ostream & out;
		const std::string elementName;
		bool containsElements;
		const char *indentation;
};

const char * getNexmlCharPref(NxsCharactersBlock::DataTypesEnum dt)
{
	if (dt == NxsCharactersBlock::standard)
		return "nex:Standard";
	if (dt == NxsCharactersBlock::dna)
		return "nex:Dna";
	if (dt == NxsCharactersBlock::rna)
		return "nex:Rna";
	if (dt == NxsCharactersBlock::protein)
		return "nex:Protein";
	if (dt == NxsCharactersBlock::continuous)
		return "nex:Continuous";
	cerr << "Mixed and Nucleotide data (int " << int(dt) <<") type not supported for nexml output\n";
	exit(2);
}

std::string getNexmlCharSeqType(NxsCharactersBlock::DataTypesEnum dt)
{
	std::string p(getNexmlCharPref(dt));
	p.append("Seqs");
	return p;
}

std::string getNexmlCharCellsType(NxsCharactersBlock::DataTypesEnum dt)
{
	std::string p(getNexmlCharPref(dt));
	p.append("Cells");
	return p;
}

class IDLabelledElement: public XMLElement
{
	public:
		IDLabelledElement(const char *elN, std::string identifier, std::string titleStr, ostream &outstream, bool contains, const char *indent, const AttributeDataVec *ovec=NULL)
			:XMLElement(elN, outstream, contains, indent)
			{
			AttributeDataVec v;
			v.push_back(AttributeData("id", identifier));
			if (!titleStr.empty())
				v.push_back(AttributeData("label", titleStr));
			if (ovec)
				v.insert(v.end(), ovec->begin(), ovec->end());
			XMLElement::open(v);
			}
};

class OtusElement: public IDLabelledElement
{
	public:
		OtusElement(std::string identifier, std::string titleStr, ostream &outstream, const AttributeDataVec *ovec=NULL)
			:IDLabelledElement("otus", identifier, titleStr, outstream, true, "  ", ovec)
			{}
};

class OtuElement: public IDLabelledElement
{
	public:
		OtuElement(std::string identifier, std::string titleStr, ostream &outstream, const AttributeDataVec *ovec=NULL)
			:IDLabelledElement("otu", identifier, titleStr, outstream, false, "    ", ovec)
			{}
};

class OtherObjLinkedElement : public XMLElement
{
	public:
		OtherObjLinkedElement(const char  * elN, std::string identifier, std::string titleStr, const char *otherAttN, std::string otherID, ostream &outstream, bool contains, const char * indent, const AttributeDataVec * att)
			:XMLElement(elN, outstream, contains, indent)
			{
			AttributeDataVec v;
			v.push_back(AttributeData("id", identifier));
			v.push_back(AttributeData(otherAttN, otherID));
			if (!titleStr.empty())
				v.push_back(AttributeData("label", titleStr));
			if (att)
				v.insert(v.end(), att->begin(), att->end());
			XMLElement::open(v);
			}
};

class OTULinkedElement: public OtherObjLinkedElement
{
	public:
		OTULinkedElement(const char  * elN, std::string identifier, std::string titleStr, std::string taxaBlockID, ostream &outstream, bool contains, const char * indent, const AttributeDataVec * att)
			:OtherObjLinkedElement(elN, identifier, titleStr, "otu", taxaBlockID, outstream, contains, indent, att)
			{}
};

class OTUSLinkedElement: public OtherObjLinkedElement
{
	public:
		OTUSLinkedElement(const char  * elN, std::string identifier, std::string titleStr, std::string taxaBlockID, ostream &outstream, bool contains, const char * indent, const AttributeDataVec * att)
			:OtherObjLinkedElement(elN, identifier, titleStr, "otus", taxaBlockID, outstream, contains, indent, att)
			{}
};
class CharactersElement: public OTUSLinkedElement
{
	public:
		CharactersElement(std::string identifier, std::string titleStr, std::string taxaBlockID, ostream &outstream, const AttributeDataVec * att)
			:OTUSLinkedElement("characters", identifier, titleStr, taxaBlockID, outstream, true, "  ", att)
			{}
};


void writeAsNexml(PublicNexusReader & nexusReader, ostream & os)
{
	os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    os << "<nex:nexml xmlns:nex=\"http://www.nexml.org/2009\" xmlns=\"http://www.nexml.org/2009\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:dcterms=\"http://purl.org/dc/terms/\" xmlns:prism=\"http://prismstandard.org/namespaces/1.2/basic/\" xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" xmlns:rdfs=\"http://www.w3.org/2000/01/rdf-schema#\" xmlns:skos=\"http://www.w3.org/2004/02/skos/core#\" xmlns:tb=\"http://purl.org/phylo/treebase/2.0/terms#\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema#\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" about=\"#S100\" generator=\"opentree.2nexml\" version=\"0.9\" >\n";
    const unsigned nTaxaBlocks = nexusReader.GetNumTaxaBlocks();
	NexmlIDStrorer memo;
	unsigned nCharBlocksRead = 0;
	unsigned nTreeBlocksRead = 0;

	for (unsigned t = 0; t < nTaxaBlocks; ++t)
		{
		const NxsTaxaBlock * tb = nexusReader.GetTaxaBlock(t);

		std::vector<const NxsAssumptionsBlock *> assumps;
		for (unsigned j= 0; j < nexusReader.GetNumAssumptionsBlocks(tb); ++j)
			assumps.push_back(nexusReader.GetAssumptionsBlock(tb, j));

		writeOTUS(os, tb, assumps, memo, t);

		const unsigned nCharBlocks = nexusReader.GetNumCharactersBlocks(tb);
		for (unsigned i = 0; i < nCharBlocks; ++i)
			{
			NxsCharactersBlock * cb = nexusReader.GetCharactersBlock(tb, i);

			assumps.clear();
			for (unsigned j= 0; j < nexusReader.GetNumAssumptionsBlocks(cb); ++j)
				assumps.push_back(nexusReader.GetAssumptionsBlock(cb, j));

			writeCharacters(os, cb, assumps, memo, nCharBlocksRead++);
			}

		const unsigned nTreesBlocks = nexusReader.GetNumTreesBlocks(tb);
		for (unsigned i = 0; i < nTreesBlocks; ++i)
			{
			NxsTreesBlock * cb = nexusReader.GetTreesBlock(tb, i);

			assumps.clear();
			for (unsigned j= 0; j < nexusReader.GetNumAssumptionsBlocks(cb); ++j)
				assumps.push_back(nexusReader.GetAssumptionsBlock(cb, j));

			writeTrees(os, cb, assumps, memo, nTreeBlocksRead++);
			}
		}

	os << "</nex:nexml>\n";
}


void writeOTUS(ostream & os, const NxsTaxaBlock *taxa, const std::vector<const NxsAssumptionsBlock *> & , NexmlIDStrorer &memo, unsigned index)
{
	if (taxa == NULL)
		return;
	TaxaBlockPtrIndPair tbp(taxa, index);
	std::string taxaBlockID = memo.getID(tbp);
	std::string title = taxa->GetTitle();
	OtusElement otus(taxaBlockID, title, os);

	const std::vector<std::string> labels = taxa->GetAllLabels();
	std::vector<std::string>::const_iterator labelIt = labels.begin();
	unsigned n = 0;
	for (; labelIt != labels.end(); ++labelIt, ++n)
		{
		std::string taxonId = memo.getID(tbp, n);
		OtuElement o(taxonId, *labelIt, os);
		}
}


void writeCharLabels(ostream & os, const NxsCharactersBlock *cb, NexmlIDStrorer &memo, unsigned index, std::vector<std::string> * indToStatesID)
{
	const std::string emptyString;
	CharBlockPtrIndPair cbp(cb, index);
	const unsigned nchars = cb->GetNumChar();
	for (unsigned i = 0; i < nchars; ++i)
		{
		std::string label = cb->GetCharLabel(i);
		if (true ) // <char  elements required now!
			{
			std::string charId = memo.getCharID(cbp, i);
			if (indToStatesID)
				{
				std::string labelToShow;
				if (label != " ")
					labelToShow = label;
				std::string statesID = (*indToStatesID)[i];
				AttributeDataVec v(1, AttributeData("states", statesID));
				IDLabelledElement c("char", charId, labelToShow, os, false, "      ", &v);
				}
			else if (label != " ")
				IDLabelledElement c2("char", charId, label, os, false, "      ");
			else
				IDLabelledElement c3("char", charId, emptyString, os, false, "      ");
			}
		}
}

std::vector<std::string> getStateLabesVec(const NxsCharactersBlock *cb, unsigned charIndex, const NxsDiscreteDatatypeMapper *mapper)
{
	const unsigned nst = mapper->GetNumStates();
	unsigned i = 0;
	std::vector<std::string> v;
	const std::string emptyString;
	for (unsigned n = 0; n < nst; ++n)
		{
		std::string l = cb->GetStateLabel(charIndex, n);
		if (l != " ")
			{
			if (i > 0)
				{
				for (unsigned j = 0; j < i; ++j)
					v.push_back(emptyString);
				i = 0;
				}
			v.push_back(l);
			}
		else
			i++;
		}
	return v;
}


const char * gDigits = "0123456789";
const char * gLetters = "abcdefghijklmnopqrstuvwxyz";

////////////////////////////////////////////////////////////////////////////////
// Creates a new states element and returns its ID.
// 	the identifier's of the each state within the states element will be the states
//	element's ID + 's' + the state index.
//	except that that:
//		the missing state's identifier will have the last state code's index
//			(mapper->GetNumStateCodes() - 1).
//		the gap state's identifier will have the next-to-last state code's index
//			(mapper->GetNumStateCodes() - 2).
////////////////////////////////////////////////////////////////////////////////
std::string writeStatesElement(const MapperStateLabelVec & mslp, ostream &os, NexmlIDStrorer &memo, unsigned charIndex, unsigned statesIndex, bool useNexusSymbols)
{
	MapperStateLabelVecIndPair m(mslp, charIndex);
	const std::string emptyString;
	const std::string identifier = memo.getID(m, statesIndex);
	IDLabelledElement statesElement("states", identifier, emptyString, os, true, "        ", NULL);
	const NxsDiscreteDatatypeMapper * mapper = mslp.first;
	std::vector<std::string> symbols;

	// figure out what symbols to use (in almost all cases this will be the NEXUS symbols
	//	but it is possible that (if the matrix was entered in TOKENS mode) there
	//	will not be NEXUS symbols for all states -- in this case we'll just numbre states.

	const NxsDiscreteStateCell nsc = (NxsDiscreteStateCell) mapper->GetNumStateCodes();
	const bool hasGap = (mapper->GetNumStatesIncludingGap() > mapper->GetNumStates());
	const NxsDiscreteStateCell endNum = (hasGap ? nsc - 2: nsc - 1); // minus one for the missing state and one more for the gap state (if used)


	int unnumberedCutoff = 10;
	if (useNexusSymbols)
		{
		for (NxsDiscreteStateCell i = 0; useNexusSymbols && i < endNum; ++i)
			{
			//cerr << i << '\n';
			std::string s = mapper->StateCodeToNexusString(i, false);
			if (s.length() != 1)
				useNexusSymbols = false;
			else
				symbols.push_back(string(1, s[0]));
			}
		if (useNexusSymbols)
			symbols.push_back(string(1, mapper->GetGapSymbol()));
		if (useNexusSymbols)
			symbols.push_back(string(1, mapper->GetMissingSymbol()));
		unnumberedCutoff = 36;
		}
	if (!useNexusSymbols)
		{
		unnumberedCutoff = 10;
		symbols.clear();
		if (nsc <= (NxsDiscreteStateCell) unnumberedCutoff)
			{
			for (int i = 0; i < 10 && (int) symbols.size() < endNum; ++i)
				symbols.push_back(std::string(1, gDigits[i]));
			for (int i = 0; i < 26 && (int) symbols.size() < endNum; ++i)
				symbols.push_back(std::string(1, gLetters[i]));
			}
		else
			{
			for (NxsDiscreteStateCell i = 0; i < endNum; ++i)
				symbols.push_back(generateID(emptyString, i));
			}
		/*
		if (hasGap)
			symbols.push_back("-");
		symbols.push_back("?");
		*/
		if (hasGap)
			symbols.push_back(generateID(emptyString, symbols.size()));
		symbols.push_back(generateID(emptyString, symbols.size()));
		}
	/// now we write the "fundamental" states
	const unsigned nst = mapper->GetNumStates();
	const std::vector<std::string> & sl = mslp.second;
	std::string stateIDpref = identifier;
	stateIDpref.append(1, 's');
	//std::string symStr(symbols.begin(), symbols.end());
	//std::cerr << "symbols = " << symStr << '\n';
	for (unsigned i = 0; i < nst; ++i)
		{
		const std::string stateID = generateID(stateIDpref, i);
		const std::string label = (i < sl.size() ? sl[i] : emptyString);
		AttributeDataVec v(1, AttributeData("symbol", symbols.at(i)));
		IDLabelledElement c("state", stateID, label, os, false, "          ", &v);
		}
	/// now we write the state sets:
	std::string gapStateID;
	if (hasGap)
		gapStateID = generateID(stateIDpref, nsc - 2);
	std::string missingStateID = generateID(stateIDpref, nsc - 1);
	/// now we deal with the gap "state"
	if (hasGap)
		{
		const std::string label("Gap");
		AttributeDataVec v(1, AttributeData("symbol", symbols[nsc - 2]));
		IDLabelledElement c("state", gapStateID, label, os, false, "          ", &v);
		}
	/// now we deal with the gap "state"
	if (hasGap)
		{
		const std::string label("Missing");
		AttributeDataVec v(1, AttributeData("symbol", symbols[nsc - 1]));
		IDLabelledElement c("state", missingStateID, label, os, false, "          ", &v);
		}
	for (int polyuncertain = 0; polyuncertain < 2; ++polyuncertain)
		{
		for (NxsDiscreteStateCell i = nst; i < endNum; ++i)
			{
			const bool isPoly = mapper->IsPolymorphic(i);
			if ((isPoly && (polyuncertain == 0)) || ((!isPoly) && (polyuncertain == 1)))
				{
				const char * elName = (isPoly ? "polymorphic_state_set" : "uncertain_state_set");
				const std::string stateID = generateID(stateIDpref, i);
				AttributeDataVec v(1, AttributeData("symbol", symbols[i]));
				IDLabelledElement stateSetElement(elName, stateID, emptyString, os, true, "          ", &v);
				const std::set<NxsDiscreteStateCell>	 & ss = mapper->GetStateSetForCode(i);
				for (std::set<NxsDiscreteStateCell>::const_iterator subStateIt = ss.begin(); subStateIt != ss.end(); ++subStateIt)
					{
					const NxsDiscreteStateCell subStateCode = *subStateIt;
					string subStateID;
					if (subStateCode < 0)
						{
						if (subStateCode == NXS_GAP_STATE_CODE)
							subStateID = gapStateID;
						else
							{
							cerr << "unexpected negative state code\n";
							exit(4);
							}
						}
					else
						subStateID = generateID(stateIDpref, (unsigned) subStateCode);
					AttributeDataVec v2(1, AttributeData("state", subStateID));
					XMLElement ss2("member", os, false, "            ", &v2);
					}
				}
			}
		}
	return identifier;
}


void writeAllStatesElements(
	ostream & os,
	const NxsCharactersBlock *cb,
	NexmlIDStrorer &memo,
	unsigned index,
	std::vector<std::string> & statesIDVec,
	const bool useNexusSymbols)
{
	const unsigned nchars = cb->GetNumChar();
	typedef std::map<MapperStateLabelVec, std::string> MSLToId;
	MSLToId mappersUsed;
	for (unsigned i = 0; i < nchars; ++i)
		{
		const NxsDiscreteDatatypeMapper * mapper = cb->GetDatatypeMapperForChar(i);
		std::vector<std::string> lv = getStateLabesVec(cb, i, mapper);
		MapperStateLabelVec mslPair(mapper, lv);
		MSLToId::const_iterator prev = mappersUsed.find(mslPair);
		const bool wse = (prev == mappersUsed.end());
		std::string sid;
		if (wse)
			{
			sid = writeStatesElement(mslPair, os, memo, index, mappersUsed.size(), useNexusSymbols);
			mappersUsed[mslPair] = sid;
			}
		else
			sid = prev->second;
		statesIDVec.push_back(sid);
		}
}
void writeCharacters(ostream & os, const NxsCharactersBlock *cb , const std::vector<const NxsAssumptionsBlock *> & , NexmlIDStrorer &memo, unsigned index)
{
	if (!cb)
		return;
	NxsTaxaBlock * taxa  = dynamic_cast<NxsTaxaBlock *>(cb->GetTaxaBlockPtr(NULL));
	if (!taxa)
		return;
	TaxaBlockPtrIndPair tbp(taxa, UINT_MAX);
	std::string taxaBlockID = memo.getID(tbp);
	CharBlockPtrIndPair cbp(cb, index);
	std::string charBlockID = memo.getID(cbp);
	std::string title = cb->GetTitle();

	NxsCharactersBlock::DataTypesEnum dt = cb->GetDataType();
	const unsigned nchars = cb->GetNumChar();

	if (dt == NxsCharactersBlock::standard || cb->GetNumUserEquates() > 0)
		{
		if (dt != NxsCharactersBlock::standard)
			{
			cerr << "Warning: user defined equates causing the coercion of " << getNexmlCharCellsType(dt) << " type to nex:StandardCells.\n";
			dt = NxsCharactersBlock::standard;
			}

		std::string dtStr = getNexmlCharCellsType(dt);
		AttributeDataVec atts(1, AttributeData("xsi:type", dtStr));
		CharactersElement charEl(charBlockID, title, taxaBlockID, os, &atts);
		std::vector<std::string> statesIDVec;
		if (true)
			{
			XMLElement format("format", os, true, "    ");
			format.open();
			writeAllStatesElements(os, cb, memo, index, statesIDVec, false);
			writeCharLabels(os, cb, memo, index, &statesIDVec);
			}
		if (true)
			{
			XMLElement mat("matrix", os, true, "    ");
			mat.open();

			const std::vector<std::string> labels = taxa->GetAllLabels();
			std::vector<std::string>::const_iterator labelIt = labels.begin();
			unsigned n = 0;
			const std::string emptyString;
			for (; labelIt != labels.end(); ++labelIt, ++n)
				{
				if (cb->TaxonIndHasData(n))
					{
					std::string rowId = memo.getID(cbp, n);
					std::string otuId = memo.getID(tbp, n);
					OTULinkedElement row("row", rowId,  emptyString, otuId, os, true, "      ", NULL);
					AttributeDataVec csAtts;
					csAtts.push_back(AttributeData("char",emptyString));
					csAtts.push_back(AttributeData("state",emptyString));
					AttributeData & charAttribute = csAtts[0];
					AttributeData & stateAttribute = csAtts[1];
					const NxsDiscreteStateRow & dataRow =  cb->GetDiscreteMatrixRow(n);
					for (unsigned k = 0; k < nchars; ++k)
						{
						charAttribute.second = memo.getCharID(cbp, k);
						const NxsDiscreteStateCell sc = dataRow[k];
						unsigned nexmlStatesIndex = 0;
						if (sc >= 0)
							nexmlStatesIndex = (unsigned) sc;
						else
							{
							const NxsDiscreteDatatypeMapper * mapper = cb->GetDatatypeMapperForChar(k);
							const unsigned nsc = mapper->GetNumStateCodes();
							if (sc == NXS_GAP_STATE_CODE)
								nexmlStatesIndex = nsc - 2;
							else if (sc == NXS_MISSING_CODE)
								nexmlStatesIndex = nsc - 1;
							else
								{
								cerr << "Unknown state code " << sc << '\n';
								exit(5);
								}
							}
						std::string stateIDpref = statesIDVec[k];
						stateIDpref.append(1, 's');
						stateAttribute.second = generateID(stateIDpref, nexmlStatesIndex);
						XMLElement("cell", os, false, "        ", &csAtts);
						}
					}
				}
			}
		}
	else
		{
		std::string dtStr = getNexmlCharSeqType(dt);
		AttributeDataVec atts(1, AttributeData("xsi:type", dtStr));
		CharactersElement charEl(charBlockID, title, taxaBlockID, os, &atts);
		if (true) // cb->HasCharLabels()) // new xsd requires format
			{
			XMLElement format("format", os, true, "    ");
			format.open();
			std::vector<std::string> statesIDVec;
			writeAllStatesElements(os, cb, memo, index, statesIDVec, true);
			writeCharLabels(os, cb, memo, index, &statesIDVec);
			}
		if (true)
			{
			XMLElement mat("matrix", os, true, "    ");
			mat.open();

			const std::vector<std::string> labels = taxa->GetAllLabels();
			std::vector<std::string>::const_iterator labelIt = labels.begin();
			unsigned n = 0;
			const std::string emptyString;
			for (; labelIt != labels.end(); ++labelIt, ++n)
				{
				if (cb->TaxonIndHasData(n))
					{
					std::string rowId = memo.getID(cbp, n);
					std::string otuId = memo.getID(tbp, n);
					OTULinkedElement row("row", rowId,  emptyString, otuId, os, true, "      ", NULL);
					if (true)
						{
						os << "        <seq>";
						cb->WriteStatesForTaxonAsNexus(os, n, 0, nchars);
						os << "</seq>\n";
						}
					}
				}
			}
		}
}

std::string writeSimpleNode(ostream & os, const NxsSimpleNode &nd, const TaxaBlockPtrIndPair & taxa, unsigned nodeIndex, NexmlIDStrorer &memo, AttributeDataVec*oatts)
{
	AttributeDataVec v;
	std::string prefix("n");
	unsigned otuInd = nd.GetTaxonIndex();
	std::string otuID;
	std::string label;
	std::string identifier = generateID(prefix, nodeIndex);
	if (otuInd != UINT_MAX)
		v.push_back(AttributeData("otu", memo.getID(taxa, otuInd)));
	else
		label = nd.GetName();
	if (oatts)
		v.insert(v.end(), oatts->begin(), oatts->end());
	IDLabelledElement nodeEl ("node", identifier, label, os, false, "      ", &v);
	return identifier;
}

std::string writeSimpleEdge(ostream & os, const NxsSimpleNode *nd, std::map<const NxsSimpleNode *, std::string>  & ndToIdMap, bool edgesAsIntegers)
{
	const NxsSimpleEdge edge = nd->GetEdgeToParent();
	bool defEdgeLen = edge.EdgeLenIsDefaultValue();
	assert(edge.GetChild() == nd);
	std::string eid(1, 'e');
	const std::string & nid = ndToIdMap[nd];
	eid.append(nid);
	NxsString lenstring;
	AttributeDataVec v;
	if (edgesAsIntegers)
		lenstring << edge.GetIntEdgeLen();
	else
		lenstring << edge.GetDblEdgeLen();
	if (!defEdgeLen)
		v.push_back(AttributeData("length", lenstring));
	v.push_back(AttributeData("target", nid));
	const NxsSimpleNode * par = edge.GetParent();
	assert(par);
	assert(ndToIdMap.find(par) != ndToIdMap.end());
	v.push_back(AttributeData("source", ndToIdMap[par]));
	IDLabelledElement edgeEl("edge", eid, std::string(), os, false, "      ", &v);
	return eid;
}
void writeTrees(ostream & os, const NxsTreesBlock *tb, const std::vector<const NxsAssumptionsBlock *> & , NexmlIDStrorer &memo, unsigned index)
{
	if (!tb)
		return;
	const NxsTaxaBlock * taxa  = dynamic_cast<const NxsTaxaBlock *>(tb->GetTaxaBlockPtr(NULL));
	if (!taxa)
		return;
	TaxaBlockPtrIndPair tbp(taxa, UINT_MAX);
	std::string taxaBlockID = memo.getID(tbp);
	TreesBlockPtrIndPair treesbp(tb, index);
	std::string treesBlockID = memo.getID(treesbp);
	std::string title = tb->GetTitle();
	const unsigned ntrees = tb->GetNumTrees();
	if (ntrees == 0)
		return;
	OTUSLinkedElement treesEl("trees", treesBlockID, title, taxaBlockID, os, true, "  ", NULL);
	tb->ProcessAllTrees();
	for (unsigned treen = 0; treen < ntrees; ++treen)
		{
		const NxsFullTreeDescription &ftd = tb->GetFullTreeDescription(treen);
		const bool edgesAsIntegers = ftd.EdgeLengthsAreAllIntegers();
		const char * treeType = (edgesAsIntegers ?  "nex:IntTree": "nex:FloatTree" );
		std::string identifier = memo.getID(treesbp, treen);
		AttributeDataVec treeAtts(1, AttributeData("xsi:type", std::string(treeType)));
		IDLabelledElement treeEl("tree", identifier, ftd.GetName(), os, true, "    ", &treeAtts);
		NxsSimpleTree tree(ftd, INT_MAX, DBL_MAX);
		std::vector<const NxsSimpleNode *> preorder = tree.GetPreorderTraversal();
		std::vector<const NxsSimpleNode *>::const_iterator ndIt = preorder.begin();
		std::map<const NxsSimpleNode *, std::string> nodeToIDMap;
		unsigned nodeIndex = 0;
		if (ndIt != preorder.end())
			{
			AttributeDataVec rootAtts;
			string rv(ftd.IsRooted() ? "true" : "false");
			rootAtts.push_back(AttributeData("root", rv));
			const NxsSimpleNode * nd = *ndIt;
			nodeToIDMap[nd] = writeSimpleNode(os, *nd, tbp, nodeIndex++, memo, &rootAtts);
			++ndIt;
			for (; ndIt != preorder.end(); ++ndIt)
				{
				nd = *ndIt;
				nodeToIDMap[nd] = writeSimpleNode(os, *nd, tbp, nodeIndex++, memo, NULL);
				}
			}
		ndIt = preorder.begin();
		nodeIndex = 0;
		if (ndIt != preorder.end())
			{
			const NxsSimpleNode * nd = *ndIt;
			const NxsSimpleEdge edge = nd->GetEdgeToParent();
			bool defEdgeLen = edge.EdgeLenIsDefaultValue();
			if (!defEdgeLen)
				{
				std::string eid(1, 'e');
				const std::string & nid = nodeToIDMap[nd];
				eid.append(nid);
				NxsString lenstring;
				AttributeDataVec v;
				if (edgesAsIntegers)
					lenstring << edge.GetIntEdgeLen();
				else
					lenstring << edge.GetDblEdgeLen();
				v.push_back(AttributeData("length", lenstring));
				v.push_back(AttributeData("target", nid));
				IDLabelledElement edgeEl("rootedge", eid, std::string(), os, false, "      ", &v);
				}
			++ndIt;
			for (; ndIt != preorder.end(); ++ndIt)
				{
				nd = *ndIt;
				writeSimpleEdge(os, nd, nodeToIDMap, edgesAsIntegers);
				}
			}
		}

}

#if defined(TO_NEXML_CONVERTER) && TO_NEXML_CONVERTER
	void	writeAsNexml(PublicNexusReader & nexusReader, ostream & os);
#endif

bool gQuietMode = false;
std::ofstream gCommonFileStream;
std::ostream * gCommonOstream = 0L;
#if defined(NCL_CONVERTER_APP) && NCL_CONVERTER_APP
	enum ExportFormatEnum
		{
		NEXUS_EXPORT_FORMAT,
		PHYLIP_EXPORT_FORMAT,
		RELAXED_PHYLIP_EXPORT_FORMAT,
		FASTA_EXPORT_FORMAT,
		NEXML_EXPORT_FORMAT,
		UNSUPPORTED_EXPORT_FORMAT
		};
	ExportFormatEnum gExportFormat = NEXML_EXPORT_FORMAT;
	std::string gExportPrefix("out");
	ExportFormatEnum readExportFormatName(const std::string &);
	void exportData(PublicNexusReader & nexusReader, MultiFormatReader::DataFormatType f, long interleavLen, std::string prefix, std::ostream *);


	ExportFormatEnum readExportFormatName(const std::string & s)
	{
		const char * gExportFormatNames[] = {   "nexus",
												"phylip",
												"relaxedphylip",
												"fasta",
												"nexml"
												};
		const unsigned gNumExportFormats = 5;

		NxsString l(s.c_str());
		NxsString::to_lower(l);
		int ind = NxsString::index_in_array(l, gExportFormatNames, gNumExportFormats);
		if (ind < 0)
			return UNSUPPORTED_EXPORT_FORMAT;
		return ExportFormatEnum(ind);
	}
	std::string gNEXUSSafeNamesToWrite;
	std::string gNEXUSSafeNamesToRead;
	bool gProduceEvenTrivalTranslation = false;

	void reverseTranslateNames(PublicNexusReader & reader, std::string filepath);
	void substituteSafeTaxaLabels(PublicNexusReader & reader, std::string filepath, bool evenTrivial);


#endif  // if defined(NCL_CONVERTER_APP) && NCL_CONVERTER_APP

bool gAltNexus = false;

void writeAsNexus(PublicNexusReader & nexusReader, ostream & os);

long gStrictLevel = 2;
bool gUnderscoresToSpaces = false;
bool gValidateInternals = true;
bool gTreesViaInMemoryStruct = true;
long gInterleaveLen = -1;
bool blocksReadInValidation = false;
bool gSuppressingNameTranslationFile = false;
bool gAllowNumericInterpretationOfTaxLabels = true;
enum ProcessActionsEnum
	{
	REPORT_BLOCKS,
	OUTPUT_NORMALIZED_NEXUS,
	OUTPUT_ANY_FORMAT,
	OUTPUT_NEXML,
	VALIDATE_ONLY
	};


void processContent(PublicNexusReader & nexusReader, ostream *os, ProcessActionsEnum currentAction);
MultiFormatReader * instantiateReader();

#	if defined(MULTIFILE_NEXUS_READER) && MULTIFILE_NEXUS_READER
	MultiFormatReader * gNexusReader = NULL;
#	endif


void reportNexusStats(const PublicNexusReader & nexusReader, ostream *os)
{
	if (!os)
		return;

	const unsigned nTaxaBlocks = nexusReader.GetNumTaxaBlocks();
	*os <<  nTaxaBlocks << " taxa block(s) read.\n";
	for (unsigned t = 0; t < nTaxaBlocks; ++t)
		{
		NxsTaxaBlock * tb = nexusReader.GetTaxaBlock(t);
		*os << "Taxa block #" << t + 1 << ".\n";
		tb->Report(*os);
		const unsigned nCharBlocks = nexusReader.GetNumCharactersBlocks(tb);
		*os <<  nCharBlocks << " Characters/Data block(s) read that link to this Taxa block.\n";
		for (unsigned i = 0; i < nCharBlocks; ++i)
			{
			NxsCharactersBlock * cb = nexusReader.GetCharactersBlock(tb, i);

			//NxsCXXDiscreteMatrix mat(*cb, true);

			*os << "Character block #" << i + 1 << " for this Taxa block.\n";
			cb->Report(*os);
			const unsigned nAssumpBlocks = nexusReader.GetNumAssumptionsBlocks(cb);
			*os <<  nAssumpBlocks << " Assumptions block(s) read that link to this Characters block.\n";
			for (unsigned j= 0; j < nAssumpBlocks; ++j)
				{
				NxsAssumptionsBlock * ab = nexusReader.GetAssumptionsBlock(cb, j);
				*os << "Assumptions block #" << j + 1 << " for this Characters block.\n";
				ab->Report(*os);
				}
			}
		const unsigned nTreesBlocks = nexusReader.GetNumTreesBlocks(tb);
		*os <<  nTreesBlocks << " Trees/Data block(s) read that link to this Taxa block.\n";
		for (unsigned i = 0; i < nTreesBlocks; ++i)
			{
			NxsTreesBlock * cb = nexusReader.GetTreesBlock(tb, i);
			*os << "Trees block #" << i + 1 << " for this Taxa block.\n";
			cb->Report(*os);
			const unsigned nAssumpBlocks = nexusReader.GetNumAssumptionsBlocks(cb);
			*os <<  nAssumpBlocks << " Assumptions block(s) read that link to this Trees block.\n";
			for (unsigned j= 0; j < nAssumpBlocks; ++j)
				{
				NxsAssumptionsBlock * ab = nexusReader.GetAssumptionsBlock(cb, j);
				*os << "Assumptions block #" << j + 1 << " for this Trees block.\n";
				ab->Report(*os);
				}
			}
		const unsigned nAssumpBlocks = nexusReader.GetNumAssumptionsBlocks(tb);
		*os <<  nAssumpBlocks << " Assumptions block(s) read that link to this Taxa block.\n";
		for (unsigned j= 0; j < nAssumpBlocks; ++j)
			{
			NxsAssumptionsBlock * ab = nexusReader.GetAssumptionsBlock(tb, j);
			*os << "Assumptions block #" << j + 1 << " for this Taxa block.\n";
			ab->Report(*os);
			}
		const unsigned nDistancesBlocks = nexusReader.GetNumDistancesBlocks(tb);
		*os <<  nDistancesBlocks << " Distances block(s) read that link to this Taxa block.\n";
		for (unsigned j= 0; j < nDistancesBlocks; ++j)
			{
			NxsDistancesBlock * ab = nexusReader.GetDistancesBlock(tb, j);
			*os << "Distances block #" << j + 1 << " for this Taxa block.\n";
			ab->Report(*os);
			}
		const unsigned nUnalignedBlocks = nexusReader.GetNumUnalignedBlocks(tb);
		*os <<  nUnalignedBlocks << " Unaligned block(s) read that link to this Taxa block.\n";
		for (unsigned j= 0; j < nUnalignedBlocks; ++j)
			{
			NxsUnalignedBlock * ab = nexusReader.GetUnalignedBlock(tb, j);
			*os << "Unaligned block #" << j + 1 << " for this Taxa block.\n";
			ab->Report(*os);
			}
		*os << "\n\n";
		}
	const unsigned nUnknown = nexusReader.GetNumUnknownBlocks();
	*os <<  nUnknown << " private block(s) read.\n";
	for (unsigned t = 0; t < nUnknown; ++t)
		{
		NxsStoreTokensBlockReader * ub = nexusReader.GetUnknownBlock(t);
		*os << "Private block #" << t + 1 << " is a " << ub->GetID() << " block.\n";
		}
}


void writeAsNexus(PublicNexusReader & nexusReader, ostream & os)
{
	BlockReaderList blocks = nexusReader.GetUsedBlocksInOrder();
	os << "#NEXUS\n";
	for (BlockReaderList::const_iterator bIt = blocks.begin(); bIt != blocks.end(); ++bIt)
		{
		NxsBlock * b = *bIt;
		if (b)
			b->WriteAsNexus(os);
		}
}



////////////////////////////////////////////////////////////////////////////////
// Takes NxsReader that has successfully read a file, and processes the
//	information stored in the reader.
//
// The caller is responsibel for calling DeleteBlocksFromFactories() to clean
//	up (if the reader uses the factory API).
////////////////////////////////////////////////////////////////////////////////
void processContent(PublicNexusReader & nexusReader, ostream *os, ProcessActionsEnum currentAction)
	{
	BlockReaderList blocks = nexusReader.GetUsedBlocksInOrder();

	if (currentAction == REPORT_BLOCKS)
		reportNexusStats(nexusReader, os);
	else if (OUTPUT_NORMALIZED_NEXUS == currentAction && os)
		{
		writeAsNexus(nexusReader, *os);
		}
	else if (OUTPUT_NEXML == currentAction && os)
		{
#		if defined(TO_NEXML_CONVERTER) && TO_NEXML_CONVERTER
			writeAsNexml(nexusReader, *os);
#		else
			cerr << "Error nexml conversion not implemented\n";
			exit(1);
#		endif
		}
	else if (OUTPUT_ANY_FORMAT == currentAction && os)
		{
#		if defined(NCL_CONVERTER_APP) && NCL_CONVERTER_APP
			if (!gNEXUSSafeNamesToRead.empty()) {
				reverseTranslateNames(nexusReader, gNEXUSSafeNamesToRead);
			}
			else if (!gNEXUSSafeNamesToWrite.empty()) {
				substituteSafeTaxaLabels(nexusReader, gNEXUSSafeNamesToWrite, gProduceEvenTrivalTranslation);
			}


			std::string fullExportPrefix;
			MultiFormatReader::DataFormatType f = MultiFormatReader::NEXUS_FORMAT;
			if (gExportFormat == NEXUS_EXPORT_FORMAT)
				exportData(nexusReader, MultiFormatReader::NEXUS_FORMAT, gInterleaveLen, gExportPrefix, gCommonOstream);
			else if (gExportFormat == NEXML_EXPORT_FORMAT)
				exportData(nexusReader, MultiFormatReader::NEXML_FORMAT, gInterleaveLen, gExportPrefix, gCommonOstream);
			else if (gExportFormat == PHYLIP_EXPORT_FORMAT) {
				fullExportPrefix = gExportPrefix;
				fullExportPrefix.append(".dna");
				f = (gInterleaveLen < 0 ? MultiFormatReader::PHYLIP_DNA_FORMAT : MultiFormatReader::INTERLEAVED_PHYLIP_DNA_FORMAT);
				exportData(nexusReader, f, gInterleaveLen, fullExportPrefix, gCommonOstream);

				fullExportPrefix = gExportPrefix;
				fullExportPrefix.append(".rna");
				f = (gInterleaveLen < 0 ? MultiFormatReader::PHYLIP_RNA_FORMAT : MultiFormatReader::INTERLEAVED_PHYLIP_RNA_FORMAT);
				exportData(nexusReader, f, gInterleaveLen, fullExportPrefix, gCommonOstream);

				fullExportPrefix = gExportPrefix;
				fullExportPrefix.append(".aa");
				f = (gInterleaveLen < 0 ? MultiFormatReader::PHYLIP_AA_FORMAT : MultiFormatReader::INTERLEAVED_PHYLIP_AA_FORMAT);
				exportData(nexusReader, f, gInterleaveLen, fullExportPrefix, gCommonOstream);

				//fullExportPrefix = gExportPrefix;
				//fullExportPrefix.append(".discrete");
				//f = (gInterleaveLen < 0 ? MultiFormatReader::PHYLIP_DISC_FORMAT : MultiFormatReader::INTERLEAVED_PHYLIP_DISC_FORMAT);
				//exportData(nexusReader, f, gInterleaveLen, fullExportPrefix, gCommonOstream);

				fullExportPrefix = gExportPrefix;
				exportData(nexusReader, MultiFormatReader::PHYLIP_TREE_FORMAT, gInterleaveLen, fullExportPrefix, gCommonOstream);
			}
			else if (gExportFormat == RELAXED_PHYLIP_EXPORT_FORMAT) {
				fullExportPrefix = gExportPrefix;
				fullExportPrefix.append(".dna");
				f = (gInterleaveLen < 0 ? MultiFormatReader::RELAXED_PHYLIP_DNA_FORMAT : MultiFormatReader::INTERLEAVED_RELAXED_PHYLIP_DNA_FORMAT);
				exportData(nexusReader, f, gInterleaveLen, fullExportPrefix, gCommonOstream);

				fullExportPrefix = gExportPrefix;
				fullExportPrefix.append(".rna");
				f = (gInterleaveLen < 0 ? MultiFormatReader::RELAXED_PHYLIP_RNA_FORMAT : MultiFormatReader::INTERLEAVED_RELAXED_PHYLIP_RNA_FORMAT);
				exportData(nexusReader, f, gInterleaveLen, fullExportPrefix, gCommonOstream);

				fullExportPrefix = gExportPrefix;
				fullExportPrefix.append(".aa");
				f = (gInterleaveLen < 0 ? MultiFormatReader::RELAXED_PHYLIP_AA_FORMAT : MultiFormatReader::INTERLEAVED_RELAXED_PHYLIP_AA_FORMAT);
				exportData(nexusReader, f, gInterleaveLen, fullExportPrefix, gCommonOstream);

				//fullExportPrefix = gExportPrefix;
				//fullExportPrefix.append(".discrete");
				//f = (gInterleaveLen < 0 ? MultiFormatReader::RELAXED_PHYLIP_DISC_FORMAT : MultiFormatReader::INTERLEAVED_RELAXED_PHYLIP_DISC_FORMAT);
				//exportData(nexusReader, f, gInterleaveLen, fullExportPrefix, gCommonOstream);

				fullExportPrefix = gExportPrefix;
				exportData(nexusReader, MultiFormatReader::RELAXED_PHYLIP_TREE_FORMAT, gInterleaveLen, fullExportPrefix, gCommonOstream);
			}
			else if (gExportFormat == FASTA_EXPORT_FORMAT) {
				fullExportPrefix = gExportPrefix;
				fullExportPrefix.append(".dna");
				exportData(nexusReader,  MultiFormatReader::FASTA_DNA_FORMAT, gInterleaveLen, fullExportPrefix, gCommonOstream);

				fullExportPrefix = gExportPrefix;
				fullExportPrefix.append(".rna");
				exportData(nexusReader,  MultiFormatReader::FASTA_RNA_FORMAT, gInterleaveLen, fullExportPrefix, gCommonOstream);

				fullExportPrefix = gExportPrefix;
				fullExportPrefix.append(".aa");
				exportData(nexusReader,  MultiFormatReader::FASTA_AA_FORMAT, gInterleaveLen, fullExportPrefix, gCommonOstream);

				fullExportPrefix = gExportPrefix;
				exportData(nexusReader, MultiFormatReader::RELAXED_PHYLIP_TREE_FORMAT, gInterleaveLen, fullExportPrefix, gCommonOstream);
			}
			else {
				cerr << "Unsupported export format requested.\n";
				exit(1);
			}
#		else
			cerr << "Exporting to any format is implemented by this executable.\n";
			exit(1);
#		endif
		}
	else if (VALIDATE_ONLY == currentAction)
		{
		if (!blocks.empty())
			blocksReadInValidation = true;
		}
	}


MultiFormatReader * instantiateReader()
{
	MultiFormatReader * nexusReader = new MultiFormatReader(-1, NxsReader::WARNINGS_TO_STDERR);
	if (gQuietMode)
		nexusReader->SetWarningOutputLevel(NxsReader::SKIPPING_CONTENT_WARNING);
	if (gStrictLevel != 2)
		nexusReader->SetWarningToErrorThreshold((int)NxsReader::FATAL_WARNING + 1 - (int) gStrictLevel);
	if (gUnderscoresToSpaces)
		nexusReader->SetCoerceUnderscoresToSpaces(true);
	NxsCharactersBlock * charsB = nexusReader->GetCharactersBlockTemplate();
	NxsDataBlock * dataB = nexusReader->GetDataBlockTemplate();
	charsB->SetAllowAugmentingOfSequenceSymbols(true);
	dataB->SetAllowAugmentingOfSequenceSymbols(true);
	if (gInterleaveLen > 0)
		{
		assert(charsB);
		charsB->SetWriteInterleaveLen(gInterleaveLen);
		dataB->SetWriteInterleaveLen(gInterleaveLen);
		}

	NxsTreesBlock * treesB = nexusReader->GetTreesBlockTemplate();
	assert(treesB);
	if (gStrictLevel < 2)
		treesB->SetAllowImplicitNames(true);
	treesB->SetWriteFromNodeEdgeDataStructure(gTreesViaInMemoryStruct);
	treesB->setValidateInternalNodeLabels(gValidateInternals);
	treesB->setAllowNumericInterpretationOfTaxLabels(gAllowNumericInterpretationOfTaxLabels);
	if (gAltNexus)
		treesB->setWriteTranslateTable(false);
	if (gStrictLevel < 2)
		{
		NxsStoreTokensBlockReader *storerB =  nexusReader->GetUnknownBlockTemplate();
		assert(storerB);
		storerB->SetTolerateEOFInBlock(true);
		}
	nexusReader->conversionOutputRecord.addNumbersToDisambiguateNames = true;

	if (gSuppressingNameTranslationFile)
		nexusReader->conversionOutputRecord.writeNameTranslationFile = false;
	return nexusReader;
}

////////////////////////////////////////////////////////////////////////////////
// Creates a NxsReader, and tries to read the file `filename`.  If the
//	read succeeds, then processContent will be called.
//	\returns 0 on success
////////////////////////////////////////////////////////////////////////////////
#if !defined(MULTIFILE_NEXUS_READER) ||  !MULTIFILE_NEXUS_READER
	int processFilepath(
		const char * filename, // name of the file to be read
		ostream * os, // output stream to use (NULL for no output). Not that cerr is used to report errors.
		MultiFormatReader::DataFormatType fmt, // enum indicating the file format to expect.
		ProcessActionsEnum currentAction) // enum that is passed on to processContent to indicate what should be done with the content of the file.
#else
	int processFilepath(
		const char * filename, // name of the file to be read
		ostream * , // output stream to use (NULL for no output). Not that cerr is used to report errors.
		MultiFormatReader::DataFormatType fmt, // enum indicating the file format to expect.
		ProcessActionsEnum ) // enum that is passed on to processContent to indicate what should be done with the content of the file.
#endif
	{
	assert(filename);
	try
		{
		MultiFormatReader * nexusReader;
#		if defined(MULTIFILE_NEXUS_READER) && MULTIFILE_NEXUS_READER
			nexusReader = gNexusReader;
#		else
			nexusReader = instantiateReader();
#		endif

		if (!gQuietMode)
			cerr << "Executing" << endl;
		try {
			nexusReader->DemoteBlocks();
			nexusReader->ReadFilepath(filename, fmt);
#			if !defined(MULTIFILE_NEXUS_READER) ||  !MULTIFILE_NEXUS_READER
				processContent(*nexusReader, os, currentAction);
#			endif
			}
		catch(...)
			{
			nexusReader->DeleteBlocksFromFactories();
#			if ! defined(MULTIFILE_NEXUS_READER) || !MULTIFILE_NEXUS_READER
				delete nexusReader;
#			endif
			throw;
			}
#		if !defined(MULTIFILE_NEXUS_READER) ||  !MULTIFILE_NEXUS_READER
			nexusReader->DeleteBlocksFromFactories();
			delete nexusReader;
#		endif
		return 0;
		}
	catch (const NxsException &x)
		{
		cerr << "Error:\n " << x.msg << endl;
		if (x.line > 0 || x.pos > 0)
			cerr << "at line " << x.line << ", column (approximately) " << x.col << " (and file position "<< x.pos << ")" << endl;
		return 2;
		}
	}

/*! \returns 0 on success*/
int readFilepathAsNEXUS(const char *filename, MultiFormatReader::DataFormatType fmt, ProcessActionsEnum currentAction)
	{
	if (!gQuietMode)
		cerr << "[Reading " << filename << "	 ]" << endl;
	try {
		ostream * outStream = 0L;
		if (currentAction != VALIDATE_ONLY)
			outStream = &cout;
		return processFilepath(filename, outStream, fmt, currentAction);

		}
	catch (...)
		{
		cerr << "Normalizing of " << filename << " failed (with an exception)" << endl;
		return 1;
		}
	}

/*! \returns 0 on success*/
int readFilesListedIsFile(const char *masterFilepath, MultiFormatReader::DataFormatType fmt, ProcessActionsEnum currentAction)
	{
	ifstream masterStream(masterFilepath, ios::binary);
	if (masterStream.bad())
		{
		cerr << "Could not open " << masterFilepath << "." << endl;
		exit(3);
		}
	char filename[1024];
	while ((!masterStream.eof())  && masterStream.good())
		{
		masterStream.getline(filename, 1024);
		if (strlen(filename) > 0 && filename[0] != '#')
			{
			int rc = readFilepathAsNEXUS(filename, fmt, currentAction);
			if (rc != 0)
				return rc;
			}
		}
	return 0;
	}

#if defined(JUST_VALIDATE_NEXUS) && JUST_VALIDATE_NEXUS
	const char * gExeName = "NEXUSvalidator";
#elif defined(JUST_REPORT_NEXUS) && JUST_REPORT_NEXUS
	const char * gExeName = "NEXUSinspector";
#elif defined(NCL_CONVERTER_APP) && NCL_CONVERTER_APP
	const char * gExeName = "NCLconverter";
#else
#	if defined(MULTIFILE_NEXUS_READER) && MULTIFILE_NEXUS_READER
		const char * gExeName = "NEXUSunion";
#	else
		const char * gExeName = "NEXUSnormalizer";
#	endif
# endif

void printHelp(ostream & out)
	{
#	if defined(JUST_VALIDATE_NEXUS) && JUST_VALIDATE_NEXUS
		out << "NEXUSvalidator takes reads a file and exits with a success (return code 0) if the file is valid.\n";
#	elif defined(JUST_REPORT_NEXUS) && JUST_REPORT_NEXUS
		out << "NEXUSinspector takes reads a file and writes a report of the content to standard out.\n";
#	elif defined(NCL_CONVERTER_APP) && NCL_CONVERTER_APP
		out << "NCLconverter takes reads a file and writes a report of the content to a file prefix (specified with the -o flag) in the chosen output format (specified with the -e flag).\n";
#	else
#		if defined(MULTIFILE_NEXUS_READER) && MULTIFILE_NEXUS_READER
			out << "NEXUSunion reads a series of NEXUS file and writes the union of all of their content to standard out (using the NEXUSnormalizer conventions of indentation and syntax).\n";
#		else
			out << "NEXUSnormalizer takes reads a file and rewrites the file to standard out with consistent indentation and syntax.\n";
#		endif
# 	endif
	out << "\nThe most common usage is simply:\n    " << gExeName << " <path to NEXUS file>\n";
	out << "\nCommand-line flags:\n\n";
	out << "    -h on the command line shows this help message\n\n";
	out << "    -q suppress NCL status messages while reading files\n\n";
	out << "    -l<path> reads a file and treats each line of the file as a path to NEXUS file\n\n";
	out << "    -a output AltNexus (no translation table in trees)\n\n";
	out << "    -x do NOT validate internal labels in trees as taxa labels\n\n";
	out << "    -X do NOT treat numbers in trees as taxon numbers, treat them as arbitrary\n        labels (should not be used with NEXUS files).\n\n";
	out << "    -s<non-negative integer> controls the NEXUS strictness level.\n";
	out << "        the default level is equivalent to -s2 invoking the program with \n";
	out << "        -s3 or a higher number will convert some warnings into fatal errors.\n";
	out << "        Running with -s1 will cause the parser to accept dangerous constructs,\n";
	out << "        and running with -s0 will cause the parser make every attempt to finish\n";
	out << "        parsing the file (warning about very serious errors).\n\n";
	out << "        Note that when -s0 strictness level is used, and the parser fails to\n";
	out << "        finish, it will often be the result of an earlier error than the \n";
	out << "        error that is reported in the last message.\n";
#	if defined(JUST_VALIDATE_NEXUS) && JUST_VALIDATE_NEXUS
		//pass
#	elif defined(JUST_REPORT_NEXUS) && JUST_REPORT_NEXUS
		//pass
#	elif defined(TO_NEXML_CONVERTER) && TO_NEXML_CONVERTER
		//pass
#	else
		out << "    -i<number> specifies the length of the interleaved pages to create\n";
#	endif
	out << "    -f<format> specifies the input file format expected:\n";
	out << "            -fnexus     NEXUS (this is also the default)\n";
	out << "            -faafasta   Amino acid data in fasta\n";
	out << "            -fdnafasta  DNA data in fasta\n";
	out << "            -frnafasta  RNA data in fasta\n";
	out << "        The complete list of format names that can follow the -f flag is:\n";
	std::vector<std::string> fmtNames =  MultiFormatReader::getFormatNames();
	for (std::vector<std::string>::const_iterator n = fmtNames.begin(); n != fmtNames.end(); ++n)
		{
		out << "            "<< *n << "\n";
		}
#	if defined(NCL_CONVERTER_APP) && NCL_CONVERTER_APP
		out << "    -e<format> specifies the output file format expected:\n";
		out << "            -enexus  \"normalized\" NEXUS output\n";
		out << "            -efasta  Character data in fasta (could result in multiple output files)\n";
		out << "            -ephylip  Trees and character data in phylip (could result in multiple output files)\n";
		out << "            -erelaxedphylip  Trees and character data in relaxed phylip (could result in multiple output files)\n";
		out << "            -enexml  nexml output (this is also the default)\n";
		out << "    -o<fn> specifies the output prefix.  An appropriate suffix and extension are added\n";
		out << "    -d<fn> specifies the single output destination. Or you can use -d- to indicate that\n";
		out << "             output should be directed to standard output.Warning use of this option may result\n";
		out << "             in an invalid output due to concatenation of separate \"blocks\" of information\n";
		out << "             into a single file!  \n";
		out << "    -u     converts underscores to spaces in formats other than NEXUS.\n";
		out << "    -y<filename> translate to \"safe\" taxon names and store the new names as a NEXUS.\n";
		out << "             file called <filename> with a TaxaAssociation block. The first taxa block\n";
		out << "             in the association block will hold the original names, and the second will\n";
		out << "             hold the \"safe\" names\n";
		out << "    -Y<filename> behaves like -y, except with -Y a translation file will be produced even";
		out << "             if the original names were already \"safe\"\n";
		out << "    -z<filename> use the NEXUS-formatted file called <filename> with a TaxaAssociation block\n";
		out << "             to restore original names.  Assumes that the first taxa block in the TaxaAssociation\n";
		out << "             block holds the original name and the second is the current name. This function\n";
		out << "             is useful for \"undoing\" the effects of the -y option.\n";
		out << "    -j     Suppress the creation of a NameTranslationFile\n";
#	endif
	}

int do_main(int argc, char *argv[])
	{
	NxsReader::setNCLCatchesSignals(true);
	MultiFormatReader::DataFormatType f(MultiFormatReader::NEXUS_FORMAT);
#	if defined(JUST_VALIDATE_NEXUS) && JUST_VALIDATE_NEXUS
		ProcessActionsEnum currentAction = VALIDATE_ONLY;
#	elif defined(JUST_REPORT_NEXUS) && JUST_REPORT_NEXUS
		ProcessActionsEnum currentAction = REPORT_BLOCKS;
#	elif defined(NCL_CONVERTER_APP) && NCL_CONVERTER_APP
		ProcessActionsEnum currentAction = OUTPUT_ANY_FORMAT;
#	elif defined(TO_NEXML_CONVERTER) && TO_NEXML_CONVERTER
		ProcessActionsEnum currentAction = OUTPUT_NEXML;
#	else
		ProcessActionsEnum currentAction = OUTPUT_NORMALIZED_NEXUS;
# 	endif

	for (int i = 1; i < argc; ++i)
		{
		const char * filepath = argv[i];
		const unsigned slen = strlen(filepath);
		if (slen < 2 || filepath[0] != '-')
			continue;
		if (filepath[1] == 'h')
			{
			printHelp(cout);
			return 1;
			}
		else if (filepath[1] == 'q')
			gQuietMode = true;
		else if (filepath[1] == 'x')
			gValidateInternals = false;
		else if (filepath[1] == 'X')
			gAllowNumericInterpretationOfTaxLabels = false;
		else if (filepath[1] == 'u')
			gUnderscoresToSpaces = true;
		else if (filepath[1] == 'j')
			gSuppressingNameTranslationFile = true;
		else if (filepath[1] == 's')
			{
			if ((slen == 2) || (!NxsString::to_long(filepath + 2, &gStrictLevel)))
				{
				cerr << "Expecting an integer after -s\n" << endl;
				printHelp(cerr);
				return 2;
				}
			}
#	if defined(JUST_VALIDATE_NEXUS) && JUST_VALIDATE_NEXUS
		//pass
#	elif defined(JUST_REPORT_NEXUS) && JUST_REPORT_NEXUS
		//pass
#	elif defined(TO_NEXML_CONVERTER) && TO_NEXML_CONVERTER
		//pass
#	else
		else if (filepath[1] == 'i')
			{
			if ((slen == 2) || (!NxsString::to_long(filepath + 2, &gInterleaveLen)) || gInterleaveLen < 1)
				{
				cerr << "Expecting a positive integer after -i\n" << endl;
				printHelp(cerr);
				return 2;
				}
			}
		else if (filepath[1] == 'a')
			{
			if ((slen != 2))
				{
				cerr << "Not expecting a value after -a\n" << endl;
				printHelp(cerr);
				return 2;
				}
			gAltNexus = true;
			}
#	endif
#	if defined(NCL_CONVERTER_APP) && NCL_CONVERTER_APP
		else if (filepath[1] == 'e') {
			if (slen > 2)
				{
				std::string efmtName(filepath + 2, slen - 2);
				gExportFormat = readExportFormatName(efmtName);
				}
			if (f == MultiFormatReader::UNSUPPORTED_FORMAT)
				{
				cerr << "Expecting a format after -e\n" << endl;
				printHelp(cerr);
				return 2;
				}
		}
		else if (filepath[1] == 'o') {
			if (slen > 2)
				{
				std::string oname(filepath + 2, slen - 2);
				gExportPrefix = oname;
				}
			if (gExportPrefix.empty())
				{
				cerr << "Expecting an output file prefix after -o\n" << endl;
				printHelp(cerr);
				return 2;
				}
		}
		else if (filepath[1] == 'd') {
			if (slen > 2)
				{
				if (gCommonOstream != 0)
					{
					cerr << "Expecting only one -d flag per invocation.\n" << endl;
					return 2;
					}
				std::string dname(filepath + 2, slen - 2);
				if (dname == "-")
					gCommonOstream = &std::cout;
				else
					{
					gCommonFileStream.open(dname.c_str());
					if (!gCommonFileStream.good())
						{
						cerr << "Error opening " << dname << ".\n" << flush;
						return 2;
						}
					if (!gQuietMode) {
						cerr << "Directing all output to " << dname << ".\n" << flush;
					}
					gCommonOstream = & gCommonFileStream;
					}
				}
			if (f == MultiFormatReader::UNSUPPORTED_FORMAT)
				{
				cerr << "Expecting an output file prefix after -o\n" << endl;
				printHelp(cerr);
				return 2;
				}
		}
		else if (filepath[1] == 'y' || filepath[1] == 'Y') {
			if (slen > 2)
				{
				std::string oname(filepath + 2, slen - 2);
				if (!gNEXUSSafeNamesToRead.empty())
					{
					cerr << "The -y and -z options cannot both be specified!\n" << endl;
					printHelp(cerr);
					return 2;
					}
				if (!gNEXUSSafeNamesToWrite.empty())
					{
					cerr << "The -y only be specified once per invocation!\n" << endl;
					printHelp(cerr);
					return 2;
					}
				gNEXUSSafeNamesToWrite = oname;
				if (gNEXUSSafeNamesToWrite.empty())
					{
					cerr << "Expecting an output file prefix after -o\n" << endl;
					printHelp(cerr);
					return 2;
					}
				}
				if (filepath[1] == 'Y')
					gProduceEvenTrivalTranslation = true;
		}
		else if (filepath[1] == 'z') {
			if (slen > 2)
				{
				std::string oname(filepath + 2, slen - 2);
				if (!gNEXUSSafeNamesToWrite.empty())
					{
					cerr << "The -y and -z options cannot both be specified!\n" << endl;
					printHelp(cerr);
					return 2;
					}
				if (!gNEXUSSafeNamesToRead.empty())
					{
					cerr << "The -z only be specified once per invocation!\n" << endl;
					printHelp(cerr);
					return 2;
					}
				gNEXUSSafeNamesToRead = oname;
				if (gNEXUSSafeNamesToRead.empty())
					{
					cerr << "Expecting an output file prefix after -o\n" << endl;
					printHelp(cerr);
					return 2;
					}
				}
		}
#	endif
		else if (filepath[1] == 'f')
			{
			f = MultiFormatReader::UNSUPPORTED_FORMAT;
			if (slen > 2)
				{
				std::string fmtName(filepath + 2, slen - 2);
				f =  MultiFormatReader::formatNameToCode(fmtName);
				if (f == MultiFormatReader::UNSUPPORTED_FORMAT)
					{
					cerr << "Unknown format \"" << fmtName << "\" after -f\n" << endl;
					printHelp(cerr);
					return 3;
					}
				}
			if (f == MultiFormatReader::UNSUPPORTED_FORMAT)
				{
				cerr << "Expecting a format after -f\n" << endl;
				printHelp(cerr);
				return 2;
				}
			}
		}

#	if defined(MULTIFILE_NEXUS_READER) && MULTIFILE_NEXUS_READER
		gNexusReader = instantiateReader();
		gNexusReader->cullIdenticalTaxaBlocks(true);
#	endif
	bool readfile = false;
	for (int i = 1; i < argc; ++i)
		{
		const char * filepath = argv[i];
		const unsigned slen = strlen(filepath);
		if (slen < 1)
			continue;
		if (strlen(filepath) > 2 && filepath[0] == '-' && filepath[1] == 'l')
			{
			readfile = true;
			int rc = readFilesListedIsFile(filepath+2, f, currentAction);
			if (rc != 0)
				return rc;
			}
		else if (filepath[0] != '-')
			{
			readfile = true;
			int rc = readFilepathAsNEXUS(filepath, f, currentAction);
			if (rc != 0)
				return rc;
			}
		}
#	if defined(MULTIFILE_NEXUS_READER) && MULTIFILE_NEXUS_READER
		if (gNexusReader)
			{
			processContent(*gNexusReader, &std::cout, OUTPUT_NORMALIZED_NEXUS);
			gNexusReader->DeleteBlocksFromFactories();
			delete gNexusReader;
			}
#	endif

	if (!readfile)
		{
		cerr << "Expecting the path to NEXUS file as the only command line argument!\n" << endl;
		printHelp(cerr);
		return 1;
		}
#	if defined(JUST_VALIDATE_NEXUS) && JUST_VALIDATE_NEXUS
		if (blocksReadInValidation)
			return  0;
		std::cerr << "No blocks read\n";
		return 1;
#	else
		return 0;
#	endif
	}

int main(int argc, char *argv[])
	{
	int rc = do_main(argc, argv);
	if (gCommonOstream != 0L && gCommonOstream == &gCommonFileStream)
		gCommonFileStream.close();
	return rc;
	}


//	Copyright (C) 2007-2008 Mark T. Holder
//
//	This file is part of NCL (Nexus Class Library).
//
//	NCL is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	NCL is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with NCL; if not, write to the Free Software Foundation, Inc.,
//	59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
