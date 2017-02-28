/********************************************************************
Created:	2012/04/15  7:14
Filename: 	RLib_RegExp.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
 *********************************************************************/
#include "RLib_RegExp.h"

#ifdef _USE_REGEXP

using namespace System::Text::RegularExpressions;
using namespace System::Text::RegularExpressions::Deelx;
/************************************************************************/
/* MatchResult                                                          */
/************************************************************************/
String MatchResult::GetGroupValue(int nGroupNumber)const
{
	return String(this->ori_string).substring(this->GetGroupStart(nGroupNumber),
											  this->GetGroupEnd(nGroupNumber) - this->GetGroupStart(nGroupNumber));
}

/************************************************************************/
/* Regex
/************************************************************************/
Regexp::Regexp(const TCHAR *pattern, int flags)
{
	Compile(pattern, RefBuffer < TCHAR >(pattern).GetSize(), flags);
}

//-------------------------------------------------------------------------

Regexp::Regexp(const TCHAR *pattern, int length, int flags)
{
	Compile(pattern, length, flags);
}

//-------------------------------------------------------------------------

RLIB_INLINE void Regexp::Compile(const TCHAR *pattern, int flags)
{
	Compile(pattern, RefBuffer < TCHAR >(pattern).GetSize(), flags);
}

//-------------------------------------------------------------------------

void Regexp::Compile(const TCHAR *pattern, int length, int flags)
{
	m_builder.Clear();
	if (pattern != 0)
	{
		m_builder.Build(RefBuffer < TCHAR >(pattern, length), flags);
	}
}

//-------------------------------------------------------------------------

RLIB_INLINE MatchResult Regexp::MatchExact(const TCHAR *tstring, CContext *pContext)const
{
	return MatchExact(tstring, RefBuffer < TCHAR >(tstring).GetSize(), pContext);
}

//-------------------------------------------------------------------------

MatchResult Regexp::MatchExact(const TCHAR *tstring, int length, CContext *pContext)const
{
	if (m_builder.m_pTopElx == 0)
	{
		return 0;
	}

	// info
	int endpos = 0;

	CContext context;
	if (pContext == 0)
	{
		pContext = &context;
	}

	pContext->m_stack.Restore(0);
	pContext->m_capturestack.Restore(0);
	pContext->m_captureindex.Restore(0);

	pContext->m_nParenZindex = 0;
	pContext->m_nLastBeginPos = -1;
	pContext->m_pMatchString = (void*)tstring;
	pContext->m_pMatchStringLength = length;

	if (m_builder.m_nFlags &RIGHTTOLEFT)
	{
		pContext->m_nBeginPos = length;
		pContext->m_nCurrentPos = length;
		endpos = 0;
	}
	else
	{
		pContext->m_nBeginPos = 0;
		pContext->m_nCurrentPos = 0;
		endpos = length;
	}

	pContext->m_captureindex.Prepare(m_builder.m_nMaxNumber, -1);
	pContext->m_captureindex[0] = 0;
	pContext->m_capturestack.Push(0);
	pContext->m_capturestack.Push(pContext->m_nCurrentPos);
	pContext->m_capturestack.Push(-1);
	pContext->m_capturestack.Push(-1);

	// match
	if (!m_builder.m_pTopElx->Match(pContext))
	{
		return 0;
	}
	else
	{
		while (pContext->m_nCurrentPos != endpos)
		{
			if (!m_builder.m_pTopElx->MatchNext(pContext))
			{
				return 0;
			}
			else
			{
				if (pContext->m_nLastBeginPos == pContext->m_nBeginPos && pContext->m_nBeginPos == pContext->m_nCurrentPos)
				{
					return 0;
				}
				else
				{
					pContext->m_nLastBeginPos = pContext->m_nCurrentPos;
				}
			}
		}

		// end pos
		pContext->m_capturestack[2] = pContext->m_nCurrentPos;

		return MatchResult(pContext, m_builder.m_nMaxNumber, tstring);
	}
}

//-------------------------------------------------------------------------

MatchResult Regexp::Match(const TCHAR *tstring, int start, CContext *pContext)const
{
	return Match(tstring, RefBuffer < TCHAR >(tstring).GetSize(), start, pContext);
}

//-------------------------------------------------------------------------

MatchResult Regexp::Match(const TCHAR *tstring, int length, int start, CContext *pContext)const
{
	if (m_builder.m_pTopElx == 0)
	{
		return 0;
	}

	CContext context;
	if (pContext == 0)
	{
		pContext = &context;
	}

	PrepareMatch(tstring, length, start, pContext);

	return Match(pContext);
}

//-------------------------------------------------------------------------

MatchResult Regexp::Match(CContext *pContext)const
{
	if (m_builder.m_pTopElx == 0)
	{
		return 0;
	}

	int endpos, delta;

	if (m_builder.m_nFlags &RIGHTTOLEFT)
	{
		endpos = -1;
		delta = -1;
	}
	else
	{
		endpos = pContext->m_pMatchStringLength + 1;
		delta = 1;
	}

	while (pContext->m_nCurrentPos != endpos)
	{
		pContext->m_captureindex.Restore(0);
		pContext->m_stack.Restore(0);
		pContext->m_capturestack.Restore(0);

		pContext->m_captureindex.Prepare(m_builder.m_nMaxNumber, -1);
		pContext->m_captureindex[0] = 0;
		pContext->m_capturestack.Push(0);
		pContext->m_capturestack.Push(pContext->m_nCurrentPos);
		pContext->m_capturestack.Push(-1);
		pContext->m_capturestack.Push(-1);

		if (m_builder.m_pTopElx->Match(pContext))
		{
			pContext->m_capturestack[2] = pContext->m_nCurrentPos;

			// zero width
			if ( /* pContext->m_nLastBeginPos == pContext->m_nBeginPos && */pContext->m_nBeginPos == pContext->m_nCurrentPos)
			{
				pContext->m_nCurrentPos += delta;
				/* continue; */
			}

			// save pos
			pContext->m_nLastBeginPos = pContext->m_nBeginPos;
			pContext->m_nBeginPos = pContext->m_nCurrentPos;

			// return
			return MatchResult(pContext, m_builder.m_nMaxNumber, (const TCHAR *)pContext->m_pMatchString);
		}
		else
		{
			pContext->m_nCurrentPos += delta;
		}
	}

	return 0;
}

//-------------------------------------------------------------------------

RLIB_INLINE CContext *Regexp::PrepareMatch(const TCHAR *tstring, int start, CContext *pContext)const
{
	return PrepareMatch(tstring, RefBuffer < TCHAR >(tstring).GetSize(), start, pContext);
}

//-------------------------------------------------------------------------

CContext *Regexp::PrepareMatch(const TCHAR *tstring, int length, int start, CContext *pContext)const
{
	if (m_builder.m_pTopElx == 0)
	{
		return 0;
	}

	if (pContext == 0)
	{
		pContext = new CContext();
	}

	pContext->m_nParenZindex = 0;
	pContext->m_nLastBeginPos = -1;
	pContext->m_pMatchString = (void*)tstring;
	pContext->m_pMatchStringLength = length;

	if (start < 0)
	{
		if (m_builder.m_nFlags &RIGHTTOLEFT)
		{
			pContext->m_nBeginPos = length;
			pContext->m_nCurrentPos = length;
		}
		else
		{
			pContext->m_nBeginPos = 0;
			pContext->m_nCurrentPos = 0;
		}
	}
	else
	{
		if (start > length)
		{
			start = length + ((m_builder.m_nFlags &RIGHTTOLEFT) ? 0 : 1);
		}

		pContext->m_nBeginPos = start;
		pContext->m_nCurrentPos = start;
	}

	return pContext;
}

//-------------------------------------------------------------------------

RLIB_INLINE int Regexp::GetNamedGroupNumber(const TCHAR *group_name)const
{
	return m_builder.GetNamedNumber(group_name);
}

//-------------------------------------------------------------------------

TCHAR *Regexp::Replace(const TCHAR *tstring, const TCHAR *replaceto, int start, int ntimes, MatchResult *result, CContext *pContext)const
{
	int result_length = 0;
	return Replace(tstring, RefBuffer < TCHAR >(tstring).GetSize(), replaceto, RefBuffer < TCHAR >(replaceto).GetSize(), result_length, start, ntimes, result, pContext);
}

//-------------------------------------------------------------------------

TCHAR *Regexp::Replace(const TCHAR *tstring, int string_length, const TCHAR *replaceto, int to_length, int &result_length, int start, int ntimes, MatchResult *remote_result, CContext *oContext)const
{
	if (m_builder.m_pTopElx == 0)
	{
		return 0;
	}

	// --- compile replace to ---

	Buffer < int > compiledto;

	static const TCHAR rtoptn[] =
	{
		T('\\'), T('$'), T('('), T('?'), T(':'), T('['), T('$'), T('&'), T('`'), T('\''), T('+'), T('_'), T('\\'), T('d'), T(']'), T('|'), T('\\'), T('{'), T('.'), T('*'), T('?'), T('\\'), T('}'), T(')'), T('\0')
	};
	static Regexp rtoreg(rtoptn);

	MatchResult local_result(0), *result = remote_result ? remote_result : &local_result;

	// prepare
	CContext *pContext = PrepareMatch(replaceto, to_length, -1, oContext);
	int lastIndex = 0, nmatch = 0;

	while (((*result) = rtoreg.Match(pContext)).IsMatched())
	{
		int delta = result->GetStart() - lastIndex;
		if (delta > 0)
		{
			compiledto.Push(lastIndex);
			compiledto.Push(delta);
		}

		lastIndex = result->GetStart();
		delta = 2;

		switch (replaceto[lastIndex + 1])
		{
			case T('$'): compiledto.Push(lastIndex);
				compiledto.Push(1);
				break;

			case T('&'): case T('`'): case T('\''): case T('+'): case T('_'): compiledto.Push(-1);
				compiledto.Push((int)replaceto[lastIndex + 1]);
				break;

			case T('{'): delta = result->GetEnd() - result->GetStart();
				nmatch = m_builder.GetNamedNumber(RefBuffer < TCHAR >(replaceto + (lastIndex + 2), delta - 3));

				if (nmatch > 0 && nmatch <= m_builder.m_nMaxNumber)
				{
					compiledto.Push(-2);
					compiledto.Push(nmatch);
				}
				else
				{
					compiledto.Push(lastIndex);
					compiledto.Push(delta);
				}
				break;

			default:
				nmatch = 0;
				for (delta = 1; delta <= 3; delta++)
				{
					TCHAR ch = replaceto[lastIndex + delta];

					if (ch < T('0') || ch > T('9'))
					{
						break;
					}

					nmatch = nmatch * 10 + (ch - T('0'));
				}

				if (nmatch > m_builder.m_nMaxNumber)
				{
					while (nmatch > m_builder.m_nMaxNumber)
					{
						nmatch /= 10;
						delta--;
					}

					if (nmatch == 0)
					{
						delta = 1;
					}
				}

				if (delta == 1)
				{
					compiledto.Push(lastIndex);
					compiledto.Push(1);
				}
				else
				{
					compiledto.Push(-2);
					compiledto.Push(nmatch);
				}
				break;
		}

		lastIndex += delta;
	}

	if (lastIndex < to_length)
	{
		compiledto.Push(lastIndex);
		compiledto.Push(to_length - lastIndex);
	}

	int rightleft = m_builder.m_nFlags &RIGHTTOLEFT;

	int tb = rightleft ? compiledto.GetSize() - 2 : 0;
	int te = rightleft ? -2 : compiledto.GetSize();
	int ts = rightleft ? -2 : 2;

	// --- compile complete ---

	int beginpos = rightleft ? string_length : 0;
	int endpos = rightleft ? 0 : string_length;

	int toIndex0 = 0;
	int toIndex1 = 0;
	int i, ntime;

	Buffer < const TCHAR * > buffer;

	// prepare
	pContext = PrepareMatch(tstring, string_length, start, pContext);
	lastIndex = beginpos;

	// Match
	for (ntime = 0; ntimes < 0 || ntime < ntimes; ntime++)
	{
		(*result) = Match(pContext);

		if (!result->IsMatched())
		{
			break;
		}

		// before
		if (rightleft)
		{
			int distance = lastIndex - result->GetEnd();
			if (distance)
			{
				buffer.Push(tstring + result->GetEnd());
				buffer.Push((const TCHAR *)distance);

				toIndex1 -= distance;
			}
			lastIndex = result->GetStart();
		}
		else
		{
			int distance = result->GetStart() - lastIndex;
			if (distance)
			{
				buffer.Push(tstring + lastIndex);
				buffer.Push((const TCHAR *)distance);

				toIndex1 += distance;
			}
			lastIndex = result->GetEnd();
		}

		toIndex0 = toIndex1;

		// middle
		for (i = tb; i != te; i += ts)
		{
			int off = compiledto[i];
			int len = compiledto[i + 1];

			const TCHAR *sub = replaceto + off;

			if (off == -1)
			{
				switch (TCHAR(len))
				{
					case T('&'): sub = tstring + result->GetStart();
						len = result->GetEnd() - result->GetStart();
						break;

					case T('`'): sub = tstring;
						len = result->GetStart();
						break;

					case T('\''): sub = tstring + result->GetEnd();
						len = string_length - result->GetEnd();
						break;

					case T('+'): for (nmatch = result->MaxGroupNumber(); nmatch >= 0; nmatch--)
					{
						if (result->GetGroupStart(nmatch) >= 0)
						{
							break;
						}
					}
								 sub = tstring + result->GetGroupStart(nmatch);
								 len = result->GetGroupEnd(nmatch) - result->GetGroupStart(nmatch);
								 break;

					case T('_'): sub = tstring;
						len = string_length;
						break;
				}
			}
			else if (off == -2)
			{
				sub = tstring + result->GetGroupStart(len);
				len = result->GetGroupEnd(len) - result->GetGroupStart(len);
			}

			buffer.Push(sub);
			buffer.Push((const TCHAR *)len);

			toIndex1 += rightleft ? (-len) : len;
		}
	}

	// after
	if (rightleft)
	{
		if (endpos < lastIndex)
		{
			buffer.Push(tstring + endpos);
			buffer.Push((const TCHAR *)(lastIndex - endpos));
		}
	}
	else
	{
		if (lastIndex < endpos)
		{
			buffer.Push(tstring + lastIndex);
			buffer.Push((const TCHAR *)(endpos - lastIndex));
		}
	}

	if (oContext == 0)
	{
		Collect(pContext);
	}

	// join string
	result_length = 0;
	for (i = 0; i < buffer.GetSize(); i += 2)
	{
		result_length += (int)buffer[i + 1];
	}

	Buffer < TCHAR > result_string;
	result_string.Prepare(result_length);
	result_string.Restore(0);

	if (rightleft)
	{
		for (i = buffer.GetSize() - 2; i >= 0; i -= 2)
		{
			result_string.append(buffer[i], (int)buffer[i + 1]);
		}
	}
	else
	{
		for (i = 0; i < buffer.GetSize(); i += 2)
		{
			result_string.append(buffer[i], (int)buffer[i + 1]);
		}
	}

	result_string.append(0);

	result->m_result.append(result_length, 3);
	result->m_result.append(ntime);

	if (rightleft)
	{
		result->m_result.append(result_length - toIndex1);
		result->m_result.append(result_length - toIndex0);
	}
	else
	{
		result->m_result.append(toIndex0);
		result->m_result.append(toIndex1);
	}

	return result_string.Detach();
}

//-------------------------------------------------------------------------

RLIB_INLINE void Regexp::Collect(TCHAR *tstring)
{
	if (tstring != 0)
	{
		RLIB_GlobalCollect(tstring);
	}
}

//-------------------------------------------------------------------------

RLIB_INLINE void Regexp::Collect(CContext *pContext)
{
	if (pContext != 0)
	{
		delete pContext;
	}
}

/************************************************************************/
/* Builder                                                              */
/************************************************************************/
Builder::Builder() : m_pattern(0, 0), prev(0, 0), curr(0, 0), next(0, 0), nex2(0, 0)
{
	Clear();
}

//-------------------------------------------------------------------------

Builder::~Builder()
{
	Clear();
}

//-------------------------------------------------------------------------

int Builder::GetNamedNumber(const RefBuffer < TCHAR >  &named)const
{
	for (int i = 0; i < m_namedlist.GetSize(); i++)
	{
		if (!((CBracketElx*)m_namedlist[i]->m_elxlist[0])->m_szNamed.CompareNoCase(named))
		{
			return ((CBracketElx*)m_namedlist[i]->m_elxlist[0])->m_nnumber;
		}
	}

	return  -3;
}

//-------------------------------------------------------------------------

ElxInterface *Builder::Build(const RefBuffer < TCHAR >  &pattern, int flags)
{
	// init
	m_pattern = pattern;
	m_nNextPos = 0;
	m_nCharsetDepth = 0;
	m_nMaxNumber = 0;
	m_nNextNamed = 0;
	m_nFlags = flags;
	m_bQuoted = 0;
	m_quote_fun = 0;

	m_grouplist.Restore(0);
	m_recursivelist.Restore(0);
	m_namedlist.Restore(0);
	m_namedbackreflist.Restore(0);
	m_namedconditionlist.Restore(0);

	int i;
	for (i = 0; i < 3; i++)
	{
		MoveNext();
	}

	// build
	m_pTopElx = BuildAlternative(flags);

	// group 0
	m_grouplist.Prepare(0);
	m_grouplist[0] = m_pTopElx;

	// append named to unnamed
	m_nGroupCount = m_grouplist.GetSize();

	m_grouplist.Prepare(m_nMaxNumber + m_namedlist.GetSize());

	for (i = 0; i < m_namedlist.GetSize(); i++)
	{
		CBracketElx *pleft = (CBracketElx*)m_namedlist[i]->m_elxlist[0];
		CBracketElx *pright = (CBracketElx*)m_namedlist[i]->m_elxlist[2];

		// append
		m_grouplist[m_nGroupCount++] = m_namedlist[i];

		if (pleft->m_nnumber > 0)
		{
			continue;
		}

		// same name
		int find_same_name = GetNamedNumber(pleft->m_szNamed);
		if (find_same_name >= 0)
		{
			pleft->m_nnumber = find_same_name;
			pright->m_nnumber = find_same_name;
		}
		else
		{
			m_nMaxNumber++;

			pleft->m_nnumber = m_nMaxNumber;
			pright->m_nnumber = m_nMaxNumber;
		}
	}

	for (i = 1; i < m_nGroupCount; i++)
	{
		CBracketElx *pleft = (CBracketElx*)((CListElx*)m_grouplist[i])->m_elxlist[0];

		if (pleft->m_nnumber > m_nMaxNumber)
		{
			m_nMaxNumber = pleft->m_nnumber;
		}
	}

	// connect recursive
	for (i = 0; i < m_recursivelist.GetSize(); i++)
	{
		if (m_recursivelist[i]->m_ndata == -3)
		{
			m_recursivelist[i]->m_ndata = GetNamedNumber(m_recursivelist[i]->m_szNamed);
		}

		if (m_recursivelist[i]->m_ndata >= 0 && m_recursivelist[i]->m_ndata <= m_nMaxNumber)
		{
			if (m_recursivelist[i]->m_ndata == 0)
			{
				m_recursivelist[i]->m_pelx = m_pTopElx;
			}
			else
				for (int j = 1; j < m_grouplist.GetSize(); j++)
				{
				if (m_recursivelist[i]->m_ndata == ((CBracketElx*)((CListElx*)m_grouplist[j])->m_elxlist[0])->m_nnumber)
				{
					m_recursivelist[i]->m_pelx = m_grouplist[j];
					break;
				}
				}
		}
	}

	// named backref
	for (i = 0; i < m_namedbackreflist.GetSize(); i++)
	{
		m_namedbackreflist[i]->m_nnumber = GetNamedNumber(m_namedbackreflist[i]->m_szNamed);
	}

	// named condition
	for (i = 0; i < m_namedconditionlist.GetSize(); i++)
	{
		int nn = GetNamedNumber(m_namedconditionlist[i]->m_szNamed);
		if (nn >= 0)
		{
			m_namedconditionlist[i]->m_nnumber = nn;
			m_namedconditionlist[i]->m_pelxask = 0;
		}
	}

	return m_pTopElx;
}

//-------------------------------------------------------------------------

void Builder::Clear()
{
	for (int i = 0; i < m_objlist.GetSize(); i++)
	{
		delete m_objlist[i];
	}

	m_objlist.Restore(0);
	m_pTopElx = 0;
	m_nMaxNumber = 0;

	memset(m_pStockElxs, 0, sizeof(m_pStockElxs));
}

//
// hex to int
//
unsigned int Builder::Hex2Int(const TCHAR *pcsz, int length, int &used)
{
	unsigned int result = 0;
	int &i = used;

	for (i = 0; i < length; i++)
	{
		if (pcsz[i] >= T('0') && pcsz[i] <= T('9'))
		{
			result = (result << 4) + (pcsz[i] - T('0'));
		}
		else if (pcsz[i] >= T('A') && pcsz[i] <= T('F'))
		{
			result = (result << 4) + (0x0A + (pcsz[i] - T('A')));
		}
		else if (pcsz[i] >= T('a') && pcsz[i] <= T('f'))
		{
			result = (result << 4) + (0x0A + (pcsz[i] - T('a')));
		}
		else
		{
			break;
		}
	}

	return result;
}

//-------------------------------------------------------------------------

RLIB_INLINE ElxInterface *Builder::Keep(ElxInterface *pelx)
{
	m_objlist.Push(pelx);
	return pelx;
}

//-------------------------------------------------------------------------

void Builder::MoveNext()
{
	// forwards
	prev = curr;
	curr = next;
	next = nex2;

	// get nex2
	while (!GetNext2()){}
	;
}

//-------------------------------------------------------------------------

int Builder::GetNext2()
{
	// check length
	if (m_nNextPos >= m_pattern.GetSize())
	{
		nex2 = TCHAR_INFO(0, 1, m_nNextPos, 0);
		return 1;
	}

	int delta = 1;
	TCHAR ch = m_pattern[m_nNextPos];

	// if quoted
	if (m_bQuoted)
	{
		if (ch == T('\\'))
		{
			if (m_pattern[m_nNextPos + 1] == T('E'))
			{
				m_quote_fun = 0;
				m_bQuoted = 0;
				m_nNextPos += 2;
				return 0;
			}
		}

		if (m_quote_fun != 0)
		{
			nex2 = TCHAR_INFO((TCHAR)(*m_quote_fun)((int)ch), 0, m_nNextPos, delta);
		}
		else
		{
			nex2 = TCHAR_INFO(ch, 0, m_nNextPos, delta);
		}

		m_nNextPos += delta;

		return 1;
	}

	// common
	switch (ch)
	{
		case T('\\'):
		{
			TCHAR ch1 = m_pattern[m_nNextPos + 1];

			// backref
			if (ch1 >= T('0') && ch1 <= T('9'))
			{
				nex2 = TCHAR_INFO(ch, 1, m_nNextPos, delta);
				break;
			}

			// escape
			delta = 2;

			switch (ch1)
			{
				case T('A'): case T('Z'): case T('z'): case T('w'): case T('W'): case T('s'): case T('S'): case T('B'): case T('d'): case T('D'): case T('k'): case T('g'): nex2 = TCHAR_INFO(ch1, 1, m_nNextPos, delta);
					break;

				case T('b'): if (m_nCharsetDepth > 0)
				{
					nex2 = TCHAR_INFO('\b', 0, m_nNextPos, delta);
				}
							 else
							 {
								 nex2 = TCHAR_INFO(ch1, 1, m_nNextPos, delta);
							 }
							 break;

							 /*
							 case T('<'):
							 case T('>'):
							 if(m_nCharsetDepth > 0)
							 nex2 = TCHAR_INFO(ch1, 0, m_nNextPos, delta);
							 else
							 nex2 = TCHAR_INFO(ch1, 1, m_nNextPos, delta);
							 break;
							 */

				case T('x'): if (m_pattern[m_nNextPos + 2] != '{')
				{
					int red = 0;
					unsigned int ch2 = Hex2Int(m_pattern.GetBuffer() + m_nNextPos + 2, 2, red);

					delta += red;

					if (red > 0)
					{
						nex2 = TCHAR_INFO(TCHAR(ch2), 0, m_nNextPos, delta);
					}
					else
					{
						nex2 = TCHAR_INFO(ch1, 0, m_nNextPos, delta);
					}

					break;
				}

				case T('u'): if (m_pattern[m_nNextPos + 2] != '{')
				{
					int red = 0;
					unsigned int ch2 = Hex2Int(m_pattern.GetBuffer() + m_nNextPos + 2, 4, red);

					delta += red;

					if (red > 0)
					{
						nex2 = TCHAR_INFO(TCHAR(ch2), 0, m_nNextPos, delta);
					}
					else
					{
						nex2 = TCHAR_INFO(ch1, 0, m_nNextPos, delta);
					}
				}
							 else
							 {
								 int red = 0;
								 unsigned int ch2 = Hex2Int(m_pattern.GetBuffer() + m_nNextPos + 3, sizeof(int) * 2, red);

								 delta += red;

								 while (m_nNextPos + delta < m_pattern.GetSize() && m_pattern.At(m_nNextPos + delta) != T('}'))
								 {
									 delta++;
								 }

								 delta++; // skip '}'

								 nex2 = TCHAR_INFO(TCHAR(ch2), 0, m_nNextPos, delta);
							 }
							 break;

				case T('a'): nex2 = TCHAR_INFO(T('\a'), 0, m_nNextPos, delta);
					break;
				case T('f'): nex2 = TCHAR_INFO(T('\f'), 0, m_nNextPos, delta);
					break;
				case T('n'): nex2 = TCHAR_INFO(T('\n'), 0, m_nNextPos, delta);
					break;
				case T('r'): nex2 = TCHAR_INFO(T('\r'), 0, m_nNextPos, delta);
					break;
				case T('t'): nex2 = TCHAR_INFO(T('\t'), 0, m_nNextPos, delta);
					break;
				case T('v'): nex2 = TCHAR_INFO(T('\v'), 0, m_nNextPos, delta);
					break;
				case T('e'): nex2 = TCHAR_INFO(TCHAR(27), 0, m_nNextPos, delta);
					break;

				case T('G'):  // skip '\G'
					if (m_nCharsetDepth > 0)
					{
						m_nNextPos += 2;
						return 0;
					}
					else
					{
						nex2 = TCHAR_INFO(ch1, 1, m_nNextPos, delta);
						break;
					}

				case T('L'): if (!m_quote_fun)
				{
					m_quote_fun = (POSIX_FUNC)::tolower;
				}

				case T('U'): if (!m_quote_fun)
				{
					m_quote_fun = (POSIX_FUNC)::toupper;
				}

				case T('Q'):
				{
					m_bQuoted = 1;
					m_nNextPos += 2;
					return 0;
				}

				case T('E'):
				{
					m_quote_fun = 0;
					m_bQuoted = 0;
					m_nNextPos += 2;
					return 0;
				}

				case 0:
					if (m_nNextPos + 1 >= m_pattern.GetSize())
					{
						delta = 1;
						nex2 = TCHAR_INFO(ch, 0, m_nNextPos, delta);
					}
					else
					{
						nex2 = TCHAR_INFO(ch1, 0, m_nNextPos, delta);
					}
					// common '\0' char
					break;

				default:
					nex2 = TCHAR_INFO(ch1, 0, m_nNextPos, delta);
					break;
			}
		}
			break;

		case T('*'): case T('+'): case T('?'): case T('.'): case T('{'): case T('}'): case T(')'): case T('|'): case T('$'): if (m_nCharsetDepth > 0)
		{
			nex2 = TCHAR_INFO(ch, 0, m_nNextPos, delta);
		}
					 else
					 {
						 nex2 = TCHAR_INFO(ch, 1, m_nNextPos, delta);
					 }
					 break;

		case T('-'): if (m_nCharsetDepth > 0)
		{
			nex2 = TCHAR_INFO(ch, 1, m_nNextPos, delta);
		}
					 else
					 {
						 nex2 = TCHAR_INFO(ch, 0, m_nNextPos, delta);
					 }
					 break;

		case T('('):
		{
			TCHAR ch1 = m_pattern[m_nNextPos + 1];
			TCHAR ch2 = m_pattern[m_nNextPos + 2];

			// skip remark
			if (ch1 == T('?') && ch2 == T('#'))
			{
				m_nNextPos += 2;
				while (m_nNextPos < m_pattern.GetSize())
				{
					if (m_pattern[m_nNextPos] == T(')'))
					{
						break;
					}

					m_nNextPos++;
				}

				if (m_pattern[m_nNextPos] == T(')'))
				{
					m_nNextPos++;

					// get next nex2
					return 0;
				}
			}
			else
			{
				if (m_nCharsetDepth > 0)
				{
					nex2 = TCHAR_INFO(ch, 0, m_nNextPos, delta);
				}
				else
				{
					nex2 = TCHAR_INFO(ch, 1, m_nNextPos, delta);
				}
			}
		}
			break;

		case T('#'): if (m_nFlags &EXTENDED)
		{
			// skip remark
			m_nNextPos++;

			while (m_nNextPos < m_pattern.GetSize())
			{
				if (m_pattern[m_nNextPos] == T('\n') || m_pattern[m_nNextPos] == T('\r'))
				{
					break;
				}

				m_nNextPos++;
			}

			// get next nex2
			return 0;
		}
					 else
					 {
						 nex2 = TCHAR_INFO(ch, 0, m_nNextPos, delta);
					 }
					 break;

		case T(' '): case T('\f'): case T('\n'): case T('\r'): case T('\t'): case T('\v'): if (m_nFlags &EXTENDED)
		{
			m_nNextPos++;

			// get next nex2
			return 0;
		}
					 else
					 {
						 nex2 = TCHAR_INFO(ch, 0, m_nNextPos, delta);
					 }
					 break;

		case T('['): if (m_nCharsetDepth == 0 || m_pattern.At(m_nNextPos + 1, 0) == T(':'))
		{
			m_nCharsetDepth++;
			nex2 = TCHAR_INFO(ch, 1, m_nNextPos, delta);
		}
					 else
					 {
						 nex2 = TCHAR_INFO(ch, 0, m_nNextPos, delta);
					 }
					 break;

		case T(']'): if (m_nCharsetDepth > 0)
		{
			m_nCharsetDepth--;
			nex2 = TCHAR_INFO(ch, 1, m_nNextPos, delta);
		}
					 else
					 {
						 nex2 = TCHAR_INFO(ch, 0, m_nNextPos, delta);
					 }
					 break;

		case T(':'): if (next == TCHAR_INFO(T('['), 1))
		{
			nex2 = TCHAR_INFO(ch, 1, m_nNextPos, delta);
		}
					 else
					 {
						 nex2 = TCHAR_INFO(ch, 0, m_nNextPos, delta);
					 }
					 break;

		case T('^'): if (m_nCharsetDepth == 0 || next == TCHAR_INFO(T('['), 1) || (curr == TCHAR_INFO(T('['), 1) && next == TCHAR_INFO(T(':'), 1)))
		{
			nex2 = TCHAR_INFO(ch, 1, m_nNextPos, delta);
		}
					 else
					 {
						 nex2 = TCHAR_INFO(ch, 0, m_nNextPos, delta);
					 }
					 break;

		case 0:
			if (m_nNextPos >= m_pattern.GetSize())
			{
				nex2 = TCHAR_INFO(ch, 1, m_nNextPos, delta);
			}
			// end of string
			else
			{
				nex2 = TCHAR_INFO(ch, 0, m_nNextPos, delta);
			}
			// common '\0' char
			break;

		default:
			nex2 = TCHAR_INFO(ch, 0, m_nNextPos, delta);
			break;
	}

	m_nNextPos += delta;

	return 1;
}

//-------------------------------------------------------------------------

ElxInterface *Builder::GetStockElx(int nStockId)
{
	ElxInterface **pStockElxs = m_pStockElxs;

	// check
	if (nStockId < 0 || nStockId >= STOCKELX_COUNT)
	{
		return GetStockElx(0);
	}

	// create if no
	if (pStockElxs[nStockId] == 0)
	{
		switch (nStockId)
		{
			case STOCKELX_EMPTY:
				pStockElxs[nStockId] = Keep(new CEmptyElx());
				break;

			case STOCKELX_WORD:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(0, 1));

				pRange->m_ranges.Push(T('A'));
				pRange->m_ranges.Push(T('Z'));
				pRange->m_ranges.Push(T('a'));
				pRange->m_ranges.Push(T('z'));
				pRange->m_ranges.Push(T('0'));
				pRange->m_ranges.Push(T('9'));
				pRange->m_chars.Push(T('_'));

				pStockElxs[nStockId] = pRange;
			}
				break;

			case STOCKELX_WORD_NOT:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(0, 0));

				pRange->m_ranges.Push(T('A'));
				pRange->m_ranges.Push(T('Z'));
				pRange->m_ranges.Push(T('a'));
				pRange->m_ranges.Push(T('z'));
				pRange->m_ranges.Push(T('0'));
				pRange->m_ranges.Push(T('9'));
				pRange->m_chars.Push(T('_'));

				pStockElxs[nStockId] = pRange;
			}
				break;

			case STOCKELX_DOT_ALL:
				pStockElxs[nStockId] = Keep(new CRangeElxT < TCHAR >(0, 0));
				break;

			case STOCKELX_DOT_NOT_ALL:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(0, 0));

				pRange->m_chars.Push(T('\n'));

				pStockElxs[nStockId] = pRange;
			}
				break;

			case STOCKELX_SPACE:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(0, 1));

				pRange->m_chars.Push(T(' '));
				pRange->m_chars.Push(T('\t'));
				pRange->m_chars.Push(T('\r'));
				pRange->m_chars.Push(T('\n'));

				pStockElxs[nStockId] = pRange;
			}
				break;

			case STOCKELX_SPACE_NOT:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(0, 0));

				pRange->m_chars.Push(T(' '));
				pRange->m_chars.Push(T('\t'));
				pRange->m_chars.Push(T('\r'));
				pRange->m_chars.Push(T('\n'));

				pStockElxs[nStockId] = pRange;
			}
				break;

			case STOCKELX_DIGITAL:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(0, 1));

				pRange->m_ranges.Push(T('0'));
				pRange->m_ranges.Push(T('9'));

				pStockElxs[nStockId] = pRange;
			}
				break;

			case STOCKELX_DIGITAL_NOT:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(0, 0));

				pRange->m_ranges.Push(T('0'));
				pRange->m_ranges.Push(T('9'));

				pStockElxs[nStockId] = pRange;
			}
				break;

			case STOCKELX_WORD_RIGHTLEFT:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(1, 1));

				pRange->m_ranges.Push(T('A'));
				pRange->m_ranges.Push(T('Z'));
				pRange->m_ranges.Push(T('a'));
				pRange->m_ranges.Push(T('z'));
				pRange->m_ranges.Push(T('0'));
				pRange->m_ranges.Push(T('9'));
				pRange->m_chars.Push(T('_'));

				pStockElxs[nStockId] = pRange;
			}
				break;

			case STOCKELX_WORD_RIGHTLEFT_NOT:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(1, 0));

				pRange->m_ranges.Push(T('A'));
				pRange->m_ranges.Push(T('Z'));
				pRange->m_ranges.Push(T('a'));
				pRange->m_ranges.Push(T('z'));
				pRange->m_ranges.Push(T('0'));
				pRange->m_ranges.Push(T('9'));
				pRange->m_chars.Push(T('_'));

				pStockElxs[nStockId] = pRange;
			}
				break;

			case STOCKELX_DOT_ALL_RIGHTLEFT:
				pStockElxs[nStockId] = Keep(new CRangeElxT < TCHAR >(1, 0));
				break;

			case STOCKELX_DOT_NOT_ALL_RIGHTLEFT:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(1, 0));

				pRange->m_chars.Push(T('\n'));

				pStockElxs[nStockId] = pRange;
			}
				break;

			case STOCKELX_SPACE_RIGHTLEFT:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(1, 1));

				pRange->m_chars.Push(T(' '));
				pRange->m_chars.Push(T('\t'));
				pRange->m_chars.Push(T('\r'));
				pRange->m_chars.Push(T('\n'));
				pRange->m_chars.Push(T('\f'));
				pRange->m_chars.Push(T('\v'));

				pStockElxs[nStockId] = pRange;
			}
				break;

			case STOCKELX_SPACE_RIGHTLEFT_NOT:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(1, 0));

				pRange->m_chars.Push(T(' '));
				pRange->m_chars.Push(T('\t'));
				pRange->m_chars.Push(T('\r'));
				pRange->m_chars.Push(T('\n'));
				pRange->m_chars.Push(T('\f'));
				pRange->m_chars.Push(T('\v'));

				pStockElxs[nStockId] = pRange;
			}
				break;

			case STOCKELX_DIGITAL_RIGHTLEFT:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(1, 1));

				pRange->m_ranges.Push(T('0'));
				pRange->m_ranges.Push(T('9'));

				pStockElxs[nStockId] = pRange;
			}
				break;

			case STOCKELX_DIGITAL_RIGHTLEFT_NOT:
			{
				CRangeElxT < TCHAR >  *pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(1, 0));

				pRange->m_ranges.Push(T('0'));
				pRange->m_ranges.Push(T('9'));

				pStockElxs[nStockId] = pRange;
			}
				break;
		}
	}

	// return
	return pStockElxs[nStockId];
}

//-------------------------------------------------------------------------

ElxInterface *Builder::BuildAlternative(int vaflags)
{
	if (curr == TCHAR_INFO(0, 1))
	{
		return GetStockElx(STOCKELX_EMPTY);
	}

	// flag instance
	int flags = vaflags;

	// first part
	ElxInterface *pAlternativeOne = BuildList(flags);

	// check alternative
	if (curr == TCHAR_INFO(T('|'), 1))
	{
		CAlternativeElx *pAlternative = (CAlternativeElx*)Keep(new CAlternativeElx());
		pAlternative->m_elxlist.Push(pAlternativeOne);

		// loop
		while (curr == TCHAR_INFO(T('|'), 1))
		{
			// skip '|' itself
			MoveNext();

			pAlternativeOne = BuildList(flags);
			pAlternative->m_elxlist.Push(pAlternativeOne);
		}

		return pAlternative;
	}

	return pAlternativeOne;
}

//-------------------------------------------------------------------------

ElxInterface *Builder::BuildList(int &flags)
{
	if (curr == TCHAR_INFO(0, 1) || curr == TCHAR_INFO(T('|'), 1) || curr == TCHAR_INFO(T(')'), 1))
	{
		return GetStockElx(STOCKELX_EMPTY);
	}

	// first
	ElxInterface *pListOne = BuildRepeat(flags);

	if (curr != TCHAR_INFO(0, 1) && curr != TCHAR_INFO(T('|'), 1) && curr != TCHAR_INFO(T(')'), 1))
	{
		CListElx *pList = (CListElx*)Keep(new CListElx(flags &RIGHTTOLEFT));
		pList->m_elxlist.Push(pListOne);

		while (curr != TCHAR_INFO(0, 1) && curr != TCHAR_INFO(T('|'), 1) && curr != TCHAR_INFO(T(')'), 1))
		{
			pListOne = BuildRepeat(flags);

			// add
			pList->m_elxlist.Push(pListOne);
		}

		return pList;
	}

	return pListOne;
}

//-------------------------------------------------------------------------

ElxInterface *Builder::BuildRepeat(int &flags)
{
	// simple
	ElxInterface *pSimple = BuildSimple(flags);

	if (curr.type == 0)
	{
		return pSimple;
	}

	// is quantifier or not
	int bIsQuantifier = 1;

	// quantifier range
	unsigned int nMin = 0, nMax = 0;

	switch (curr.ch)
	{
		case T('{'):
		{
			Buffer < char > re;

			// skip '{'
			MoveNext();

			// copy
			while (curr != TCHAR_INFO(0, 1) && curr != TCHAR_INFO(T('}'), 1))
			{
				re.append(((curr.ch &(TCHAR)0xffU) == curr.ch) ? (char)curr.ch : 0, 1);
				MoveNext();
			}

			// skip '}'
			MoveNext();

			// read
			int red;
			char *str = re.GetBuffer();

			if (!ReadDec(str, nMin))
			{
				red = 0;
			}
			else if (*str != ',')
			{
				red = 1;
			}
			else
			{
				str++;

				if (!ReadDec(str, nMax))
				{
					red = 2;
				}
				else
				{
					red = 3;
				}
			}

			// check
			if (red <= 1)
			{
				nMax = nMin;
			}
			if (red == 2)
			{
				nMax = INT_MAX;
			}
			if (nMax < nMin)
			{
				nMax = nMin;
			}
		}
			break;

		case T('?'): nMin = 0;
			nMax = 1;

			// skip '?'
			MoveNext();
			break;

		case T('*'): nMin = 0;
			nMax = INT_MAX;

			// skip '*'
			MoveNext();
			break;

		case T('+'): nMin = 1;
			nMax = INT_MAX;

			// skip '+'
			MoveNext();
			break;

		default:
			bIsQuantifier = 0;
			break;
	}

	// do quantify
	if (bIsQuantifier)
	{
		// 0 times
		if (nMax == 0)
		{
			return GetStockElx(STOCKELX_EMPTY);
		}

		// fixed times
		if (nMin == nMax)
		{
			if (curr == TCHAR_INFO(T('?'), 1) || curr == TCHAR_INFO(T('+'), 1))
			{
				MoveNext();
			}

			return Keep(new CRepeatElx(pSimple, nMin));
		}

		// range times
		if (curr == TCHAR_INFO(T('?'), 1))
		{
			MoveNext();
			return Keep(new CReluctantElx(pSimple, nMin, nMax));
		}
		else if (curr == TCHAR_INFO(T('+'), 1))
		{
			MoveNext();
			return Keep(new CPossessiveElx(pSimple, nMin, nMax));
		}
		else
		{
			return Keep(new CGreedyElx(pSimple, nMin, nMax));
		}
	}

	return pSimple;
}

//-------------------------------------------------------------------------

ElxInterface *Builder::BuildSimple(int &flags)
{
	Buffer < TCHAR > fixed;

	while (curr != TCHAR_INFO(0, 1))
	{
		if (curr.type == 0)
		{
			if (next == TCHAR_INFO(T('{'), 1) || next == TCHAR_INFO(T('?'), 1) || next == TCHAR_INFO(T('*'), 1) || next == TCHAR_INFO(T('+'), 1))
			{
				if (fixed.GetSize() == 0)
				{
					fixed.append(curr.ch, 1);
					MoveNext();
				}

				break;
			}
			else
			{
				fixed.append(curr.ch, 1);
				MoveNext();
			}
		}
		else if (curr.type == 1)
		{
			TCHAR vch = curr.ch;

			// end of simple
			if (vch == T(')') || vch == T('|'))
			{
				break;
			}

			// has fixed already
			if (fixed.GetSize() > 0)
			{
				break;
			}

			// left parentheses
			if (vch == T('('))
			{
				return BuildRecursive(flags);
			}

			// char set
			if (vch == T('[') || vch == T('.') || vch == T('w') || vch == T('W') || vch == T('s') || vch == T('S') || vch == T('d') || vch == T('D'))
			{
				return BuildCharset(flags);
			}

			// boundary
			if (vch == T('^') || vch == T('$') || vch == T('A') || vch == T('Z') || vch == T('z') || vch == T('b') || vch == T('B') || vch == T('G') // vch == T('<') || vch == T('>')
				)
			{
				return BuildBoundary(flags);
			}

			// backref
			if (vch == T('\\') || vch == T('k') || vch == T('g'))
			{
				return BuildBackref(flags);
			}

			// treat vchar as char
			fixed.append(curr.ch, 1);
			MoveNext();
		}
	}

	if (fixed.GetSize() > 0)
	{
		return Keep(new CStringElxT < TCHAR >(fixed.GetBuffer(), fixed.GetSize(), flags &RIGHTTOLEFT, flags &IGNORECASE));
	}
	else
	{
		return GetStockElx(STOCKELX_EMPTY);
	}
}

//-------------------------------------------------------------------------

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))

ElxInterface *Builder::BuildCharset(int &flags)
{
	// char
	TCHAR ch = curr.ch;

	// skip
	MoveNext();

	switch (ch)
	{
		case T('.'): return GetStockElx(flags &RIGHTTOLEFT ? ((flags &SINGLELINE) ? STOCKELX_DOT_ALL_RIGHTLEFT : STOCKELX_DOT_NOT_ALL_RIGHTLEFT) : ((flags &SINGLELINE) ? STOCKELX_DOT_ALL : STOCKELX_DOT_NOT_ALL));

		case T('w'): return GetStockElx(flags &RIGHTTOLEFT ? STOCKELX_WORD_RIGHTLEFT : STOCKELX_WORD);

		case T('W'): return GetStockElx(flags &RIGHTTOLEFT ? STOCKELX_WORD_RIGHTLEFT_NOT : STOCKELX_WORD_NOT);

		case T('s'): return GetStockElx(flags &RIGHTTOLEFT ? STOCKELX_SPACE_RIGHTLEFT : STOCKELX_SPACE);

		case T('S'): return GetStockElx(flags &RIGHTTOLEFT ? STOCKELX_SPACE_RIGHTLEFT_NOT : STOCKELX_SPACE_NOT);

		case T('d'): return GetStockElx(flags &RIGHTTOLEFT ? STOCKELX_DIGITAL_RIGHTLEFT : STOCKELX_DIGITAL);

		case T('D'): return GetStockElx(flags &RIGHTTOLEFT ? STOCKELX_DIGITAL_RIGHTLEFT_NOT : STOCKELX_DIGITAL_NOT);

		case T('['):
		{
			CRangeElxT < TCHAR >  *pRange;

			// create
			if (curr == TCHAR_INFO(T(':'), 1))
			{
				Buffer < char > posix;

				do
				{
					posix.append(((curr.ch &(TCHAR)0xffU) == curr.ch) ? (char)curr.ch : 0, 1);
					MoveNext();
				} while (curr.ch != TCHAR(0) && curr != TCHAR_INFO(T(']'), 1));

				MoveNext(); // skip ']'

				// posix
				return Keep(new CPosixElxT < TCHAR >(posix.GetBuffer(), flags &RIGHTTOLEFT));
			}
			else if (curr == TCHAR_INFO(T('^'), 1))
			{
				MoveNext(); // skip '^'
				pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(flags &RIGHTTOLEFT, 0));
			}
			else
			{
				pRange = (CRangeElxT < TCHAR > *)Keep(new CRangeElxT < TCHAR >(flags &RIGHTTOLEFT, 1));
			}

			// parse
			while (curr != TCHAR_INFO(0, 1) && curr != TCHAR_INFO(T(']'), 1))
			{
				ch = curr.ch;

				if (curr.type == 1 && (ch == T('.') || ch == T('w') || ch == T('W') || ch == T('s') || ch == T('S') || ch == T('d') || ch == T('D') || (ch == T('[') && next == TCHAR_INFO(T(':'), 1))))
				{
					pRange->m_embeds.Push(BuildCharset(flags));
				}
				else if (next == TCHAR_INFO(T('-'), 1) && nex2.type == 0)
				{
					pRange->m_ranges.Push(ch);
					pRange->m_ranges.Push(nex2.ch);

					// next
					MoveNext();
					MoveNext();
					MoveNext();
				}
				else
				{
					pRange->m_chars.Push(ch);

					// next
					MoveNext();
				}
			}

			// skip ']'
			MoveNext();

			if (flags &IGNORECASE)
			{
				Buffer < TCHAR >  &ranges = pRange->m_ranges;
				int i, oldcount = ranges.GetSize() / 2;

				for (i = 0; i < oldcount; i++)
				{
					TCHAR newmin, newmax;

					if (ranges[i * 2] <= T('Z') && ranges[i * 2 + 1] >= T('A'))
					{
						newmin = tolower(max(T('A'), ranges[i * 2]));
						newmax = tolower(min(T('Z'), ranges[i * 2 + 1]));

						if (newmin < ranges[i * 2] || newmax > ranges[i * 2 + 1])
						{
							ranges.Push(newmin);
							ranges.Push(newmax);
						}
					}

					if (ranges[i * 2] <= T('z') && ranges[i * 2 + 1] >= T('a'))
					{
						newmin = toupper(max(T('a'), ranges[i * 2]));
						newmax = toupper(min(T('z'), ranges[i * 2 + 1]));

						if (newmin < ranges[i * 2] || newmax > ranges[i * 2 + 1])
						{
							ranges.Push(newmin);
							ranges.Push(newmax);
						}
					}
				}

				Buffer < TCHAR >  &chars = pRange->m_chars;
				oldcount = chars.GetSize();
				for (i = 0; i < oldcount; i++)
				{
					if (isupper(chars[i]) && !pRange->IsContainChar(tolower(chars[i])))
					{
						chars.Push(tolower(chars[i]));
					}

					if (islower(chars[i]) && !pRange->IsContainChar(toupper(chars[i])))
					{
						chars.Push(toupper(chars[i]));
					}
				}
			}

			return pRange;
		}
	}

	return GetStockElx(STOCKELX_EMPTY);
}

//-------------------------------------------------------------------------

ElxInterface *Builder::BuildRecursive(int &flags)
{
	// skip '('
	MoveNext();

	if (curr == TCHAR_INFO(T('?'), 1))
	{
		ElxInterface *pElx = 0;

		// skip '?'
		MoveNext();

		int bNegative = 0;
		TCHAR named_end = T('>');

		switch (curr.ch)
		{
			case T('!'): bNegative = 1;

			case T('='):
			{
				MoveNext(); // skip '!' or '='
				pElx = Keep(new CAssertElx(BuildAlternative(flags &~RIGHTTOLEFT), !bNegative));
			}
				break;

			case T('<'): switch (next.ch)
			{
				case T('!'): bNegative = 1;

				case T('='): MoveNext(); // skip '<'
					MoveNext(); // skip '!' or '='
					{
						pElx = Keep(new CAssertElx(BuildAlternative(flags | RIGHTTOLEFT), !bNegative));
					}
					break;

				default:
					// named group
					break;
			}
						 // break if assertion // else named
						 if (pElx != 0)
						 {
							 break;
						 }

			case T('P'): if (curr.ch == T('P'))
			{
				MoveNext();
			}
						 // skip 'P'

			case T('\''): if (curr.ch == T('<'))
			{
				named_end = T('>');
			}
						  else if (curr.ch == T('\''))
						  {
							  named_end = T('\'');
						  }
						  MoveNext(); // skip '<' or '\''
						  {
							  // named number
							  int nThisBackref = m_nNextNamed++;

							  CListElx *pList = (CListElx*)Keep(new CListElx(flags &RIGHTTOLEFT));
							  CBracketElx *pleft = (CBracketElx*)Keep(new CBracketElx(-1, flags &RIGHTTOLEFT ? 1 : 0));
							  CBracketElx *pright = (CBracketElx*)Keep(new CBracketElx(-1, flags &RIGHTTOLEFT ? 0 : 1));

							  // save name
							  Buffer < TCHAR >  &name = pleft->m_szNamed;
							  Buffer < char > num;

							  while (curr.ch != TCHAR(0) && curr.ch != named_end)
							  {
								  name.append(curr.ch, 1);
								  num.append(((curr.ch &(TCHAR)0xffU) == curr.ch) ? (char)curr.ch : 0, 1);
								  MoveNext();
							  }
							  MoveNext(); // skip '>' or '\''

							  // check <num>
							  unsigned int number;
							  char *str = num.GetBuffer();

							  if (ReadDec(str, number) ? (*str == '\0') : 0)
							  {
								  pleft->m_nnumber = number;
								  pright->m_nnumber = number;

								  name.Release();
							  }

							  // left, center, right
							  pList->m_elxlist.Push(pleft);
							  pList->m_elxlist.Push(BuildAlternative(flags));
							  pList->m_elxlist.Push(pright);

							  // for recursive
							  m_namedlist.Prepare(nThisBackref);
							  m_namedlist[nThisBackref] = pList;

							  pElx = pList;
						  }
						  break;

			case T('>'):
			{
				MoveNext(); // skip '>'
				pElx = Keep(new CIndependentElx(BuildAlternative(flags)));
			}
				break;

			case T('R'): MoveNext(); // skip 'R'
				while (curr.ch != TCHAR(0) && isspace(curr.ch))
				{
					MoveNext();
				}
				// skip space

				if (curr.ch == T('<') || curr.ch == T('\''))
				{
					named_end = curr.ch == T('<') ? T('>') : T('\'');
					CDelegateElx *pDelegate = (CDelegateElx*)Keep(new CDelegateElx(-3));

					MoveNext(); // skip '<' or '\\'

					// save name
					Buffer < TCHAR >  &name = pDelegate->m_szNamed;
					Buffer < char > num;

					while (curr.ch != TCHAR(0) && curr.ch != named_end)
					{
						name.append(curr.ch, 1);
						num.append(((curr.ch &(TCHAR)0xffU) == curr.ch) ? (char)curr.ch : 0, 1);
						MoveNext();
					}
					MoveNext(); // skip '>' or '\''

					// check <num>
					unsigned int number;
					char *str = num.GetBuffer();

					if (ReadDec(str, number) ? (*str == '\0') : 0)
					{
						pDelegate->m_ndata = number;
						name.Release();
					}

					m_recursivelist.Push(pDelegate);
					pElx = pDelegate;
				}
				else
				{
					Buffer < char > rto;
					while (curr.ch != TCHAR(0) && curr.ch != T(')'))
					{
						rto.append(((curr.ch &(TCHAR)0xffU) == curr.ch) ? (char)curr.ch : 0, 1);
						MoveNext();
					}

					unsigned int rtono = 0;
					char *str = rto.GetBuffer();
					ReadDec(str, rtono);

					CDelegateElx *pDelegate = (CDelegateElx*)Keep(new CDelegateElx(rtono));

					m_recursivelist.Push(pDelegate);
					pElx = pDelegate;
				}
				break;

			case T('('):
			{
				CConditionElx *pConditionElx = (CConditionElx*)Keep(new CConditionElx());

				// condition
				ElxInterface * &pCondition = pConditionElx->m_pelxask;

				if (next == TCHAR_INFO(T('?'), 1))
				{
					pCondition = BuildRecursive(flags);
				}
				else
					// named, assert or number
				{
					MoveNext(); // skip '('
					int pos0 = curr.pos;

					// save elx condition
					pCondition = Keep(new CAssertElx(BuildAlternative(flags), 1));

					// save name
					pConditionElx->m_szNamed.append(m_pattern.GetBuffer() + pos0, curr.pos - pos0, 1);

					// save number
					Buffer < char > numstr;
					while (pos0 < curr.pos)
					{
						TCHAR ch = m_pattern[pos0];
						numstr.append(((ch &(TCHAR)0xffU) == ch) ? (char)ch : 0, 1);
						pos0++;
					}

					unsigned int number;
					char *str = numstr.GetBuffer();

					// valid group number
					if (ReadDec(str, number) ? (*str == '\0') : 0)
					{
						pConditionElx->m_nnumber = number;
						pCondition = 0;
					}
					else
						// maybe elx, maybe named
					{
						pConditionElx->m_nnumber = -1;
						m_namedconditionlist.Push(pConditionElx);
					}

					MoveNext(); // skip ')'
				}

				// alternative
					{
						int newflags = flags;

						pConditionElx->m_pelxyes = BuildList(newflags);
					}

				if (curr.ch == T('|'))
				{
					MoveNext(); // skip '|'

					pConditionElx->m_pelxno = BuildAlternative(flags);
				}
				else
				{
					pConditionElx->m_pelxno = 0;
				}

				pElx = pConditionElx;
			}
				break;

			default:
				while (curr.ch != TCHAR(0) && isspace(curr.ch))
				{
					MoveNext();
				}
				// skip space

				if (curr.ch >= T('0') && curr.ch <= T('9'))
					// recursive (?1) => (?R1)
				{
					Buffer < char > rto;
					while (curr.ch != TCHAR(0) && curr.ch != T(')'))
					{
						rto.append(((curr.ch &(TCHAR)0xffU) == curr.ch) ? (char)curr.ch : 0, 1);
						MoveNext();
					}

					unsigned int rtono = 0;
					char *str = rto.GetBuffer();
					ReadDec(str, rtono);

					CDelegateElx *pDelegate = (CDelegateElx*)Keep(new CDelegateElx(rtono));

					m_recursivelist.Push(pDelegate);
					pElx = pDelegate;
				}
				else
				{
					// flag
					int newflags = flags;
					while (curr != TCHAR_INFO(0, 1) && curr.ch != T(':') && curr.ch != T(')') && curr != TCHAR_INFO(T('('), 1))
					{
						int tochange = 0;

						switch (curr.ch)
						{
							case T('i'): case T('I'): tochange = IGNORECASE;
								break;

							case T('s'): case T('S'): tochange = SINGLELINE;
								break;

							case T('m'): case T('M'): tochange = MULTILINE;
								break;

							case T('g'): case T('G'): tochange = GLOBAL;
								break;

							case T('-'): bNegative = 1;
								break;
						}

						if (bNegative)
						{
							newflags &= ~tochange;
						}
						else
						{
							newflags |= tochange;
						}

						// move to next char
						MoveNext();
					}

					if (curr.ch == T(':') || curr == TCHAR_INFO(T('('), 1))
					{
						// skip ':'
						if (curr.ch == T(':'))
						{
							MoveNext();
						}

						pElx = BuildAlternative(newflags);
					}
					else
					{
						// change parent flags
						flags = newflags;

						pElx = GetStockElx(STOCKELX_EMPTY);
					}
				}
				break;
		}

		MoveNext(); // skip ')'

		return pElx;
	}
	else
	{
		// group and number
		CListElx *pList = (CListElx*)Keep(new CListElx(flags &RIGHTTOLEFT));
		int nThisBackref = ++m_nMaxNumber;

		// left, center, right
		pList->m_elxlist.Push(Keep(new CBracketElx(nThisBackref, flags &RIGHTTOLEFT ? 1 : 0)));
		pList->m_elxlist.Push(BuildAlternative(flags));
		pList->m_elxlist.Push(Keep(new CBracketElx(nThisBackref, flags &RIGHTTOLEFT ? 0 : 1)));

		// for recursive
		m_grouplist.Prepare(nThisBackref);
		m_grouplist[nThisBackref] = pList;

		// right
		MoveNext(); // skip ')' 

		return pList;
	}
}

//-------------------------------------------------------------------------

ElxInterface *Builder::BuildBoundary(int &flags)
{
	// char
	TCHAR ch = curr.ch;

	// skip
	MoveNext();

	switch (ch)
	{
		case T('^'): return Keep(new CBoundaryElxT < TCHAR >((flags &MULTILINE) ? BOUNDARY_LINE_BEGIN : BOUNDARY_FILE_BEGIN));

		case T('$'): return Keep(new CBoundaryElxT < TCHAR >((flags &MULTILINE) ? BOUNDARY_LINE_END : BOUNDARY_FILE_END));

		case T('b'): return Keep(new CBoundaryElxT < TCHAR >(BOUNDARY_WORD_EDGE));

		case T('B'): return Keep(new CBoundaryElxT < TCHAR >(BOUNDARY_WORD_EDGE, 0));

		case T('A'): return Keep(new CBoundaryElxT < TCHAR >(BOUNDARY_FILE_BEGIN));

		case T('Z'): return Keep(new CBoundaryElxT < TCHAR >(BOUNDARY_FILE_END_N));

		case T('z'): return Keep(new CBoundaryElxT < TCHAR >(BOUNDARY_FILE_END));

		case T('G'): if (flags &GLOBAL)
		{
			return Keep(new CGlobalElx());
		}
					 else
					 {
						 return GetStockElx(STOCKELX_EMPTY);
					 }

		default:
			return GetStockElx(STOCKELX_EMPTY);
	}
}

//-------------------------------------------------------------------------

ElxInterface *Builder::BuildBackref(int &flags)
{
	// skip '\\' or '\k' or '\g'
	MoveNext();

	if (curr.ch == T('<') || curr.ch == T('\''))
	{
		TCHAR named_end = curr.ch == T('<') ? T('>') : T('\'');
		CBackrefElxT < TCHAR >  *pbackref = (CBackrefElxT < TCHAR > *)Keep(new CBackrefElxT < TCHAR >(-1, flags &RIGHTTOLEFT, flags &IGNORECASE));

		MoveNext(); // skip '<' or '\''

		// save name
		Buffer < TCHAR >  &name = pbackref->m_szNamed;
		Buffer < char > num;

		while (curr.ch != TCHAR(0) && curr.ch != named_end)
		{
			name.append(curr.ch, 1);
			num.append(((curr.ch &(TCHAR)0xffU) == curr.ch) ? (char)curr.ch : 0, 1);
			MoveNext();
		}
		MoveNext(); // skip '>' or '\''

		// check <num>
		unsigned int number;
		char *str = num.GetBuffer();

		if (ReadDec(str, number) ? (*str == '\0') : 0)
		{
			pbackref->m_nnumber = number;
			name.Release();
		}
		else
		{
			m_namedbackreflist.Push(pbackref);
		}

		return pbackref;
	}
	else
	{
		unsigned int nbackref = 0;

		for (int i = 0; i < 3; i++)
		{
			if (curr.ch >= T('0') && curr.ch <= T('9'))
			{
				nbackref = nbackref * 10 + (curr.ch - T('0'));
			}
			else
			{
				break;
			}

			MoveNext();
		}

		return Keep(new CBackrefElxT < TCHAR >(nbackref, flags &RIGHTTOLEFT, flags &IGNORECASE));
	}
}

//-------------------------------------------------------------------------

int Builder::ReadDec(char * &str, unsigned int &dec)
{
	int s = 0;
	while (str[s] != 0 && isspace((unsigned char)str[s]))
	{
		s++;
	}

	if (str[s] < '0' || str[s] > '9')
	{
		return 0;
	}

	dec = 0;
	unsigned int i;

	for (i = s; i < sizeof(TCHAR) * 3 + s; i++)
	{
		if (str[i] >= '0' && str[i] <= '9')
		{
			dec = dec * 10 + (str[i] - '0');
		}
		else
		{
			break;
		}
	}

	while (str[i] != 0 && isspace((unsigned char)str[i]))
	{
		i++;
	}
	str += i;

	return 1;
}

#endif // _USE_REGEXP