/*
FILE:     ParentChild.C
*/
/*
VERSION:  7.105
*/
/*
DATE:     10/21/2013
*/
/*
  Comments and Questions to: sw-help@rcsb.rutgers.edu
*/
/*
COPYRIGHT 1999-2013 Rutgers - The State University of New Jersey

This software is provided WITHOUT WARRANTY OF MERCHANTABILITY OR
FITNESS FOR A PARTICULAR PURPOSE OR ANY OTHER WARRANTY, EXPRESS OR
IMPLIED.  RUTGERS MAKE NO REPRESENTATION OR WARRANTY THAT THE
SOFTWARE WILL NOT INFRINGE ANY PATENT, COPYRIGHT OR OTHER
PROPRIETARY RIGHT.

The user of this software shall indemnify, hold harmless and defend
Rutgers, its governors, trustees, officers, employees, students,
agents and the authors against any and all claims, suits,
losses, liabilities, damages, costs, fees, and expenses including
reasonable attorneys' fees resulting from or arising out of the
use of this software.  This indemnification shall include, but is
not limited to, any and all claims alleging products liability.
*/
/*
               RCSB PDB SOFTWARE LICENSE AGREEMENT

BY CLICKING THE ACCEPTANCE BUTTON OR INSTALLING OR USING 
THIS "SOFTWARE, THE INDIVIDUAL OR ENTITY LICENSING THE  
SOFTWARE ("LICENSEE") IS CONSENTING TO BE BOUND BY AND IS 
BECOMING A PARTY TO THIS AGREEMENT.  IF LICENSEE DOES NOT 
AGREE TO ALL OF THE TERMS OF THIS AGREEMENT
THE LICENSEE MUST NOT INSTALL OR USE THE SOFTWARE.

1. LICENSE AGREEMENT

This is a license between you ("Licensee") and the Protein Data Bank (PDB) 
at Rutgers, The State University of New Jersey (hereafter referred to 
as "RUTGERS").   The software is owned by RUTGERS and protected by 
copyright laws, and some elements are protected by laws governing 
trademarks, trade dress and trade secrets, and may be protected by 
patent laws. 

2. LICENSE GRANT

RUTGERS grants you, and you hereby accept, non-exclusive, royalty-free 
perpetual license to install, use, modify, prepare derivative works, 
incorporate into other computer software, and distribute in binary 
and source code format, or any derivative work thereof, together with 
any associated media, printed materials, and on-line or electronic 
documentation (if any) provided by RUTGERS (collectively, the "SOFTWARE"), 
subject to the following terms and conditions: (i) any distribution 
of the SOFTWARE shall bind the receiver to the terms and conditions 
of this Agreement; (ii) any distribution of the SOFTWARE in modified 
form shall clearly state that the SOFTWARE has been modified from 
the version originally obtained from RUTGERS.  

2. COPYRIGHT; RETENTION OF RIGHTS.  

The above license grant is conditioned on the following: (i) you must 
reproduce all copyright notices and other proprietary notices on any 
copies of the SOFTWARE and you must not remove such notices; (ii) in 
the event you compile the SOFTWARE, you will include the copyright 
notice with the binary in such a manner as to allow it to be easily 
viewable; (iii) if you incorporate the SOFTWARE into other code, you 
must provide notice that the code contains the SOFTWARE and include 
a copy of the copyright notices and other proprietary notices.  All 
copies of the SOFTWARE shall be subject to the terms of this Agreement.  

3. NO MAINTENANCE OR SUPPORT; TREATMENT OF ENHANCEMENTS 

RUTGERS is under no obligation whatsoever to: (i) provide maintenance 
or support for the SOFTWARE; or (ii) to notify you of bug fixes, patches, 
or upgrades to the features, functionality or performance of the 
SOFTWARE ("Enhancements") (if any), whether developed by RUTGERS 
or third parties.  If, in its sole discretion, RUTGERS makes an 
Enhancement available to you and RUTGERS does not separately enter 
into a written license agreement with you relating to such bug fix, 
patch or upgrade, then it shall be deemed incorporated into the SOFTWARE 
and subject to this Agreement. You are under no obligation whatsoever 
to provide any Enhancements to RUTGERS or the public that you may 
develop over time; however, if you choose to provide your Enhancements 
to RUTGERS, or if you choose to otherwise publish or distribute your 
Enhancements, in source code form without contemporaneously requiring 
end users or RUTGERS to enter into a separate written license agreement 
for such Enhancements, then you hereby grant RUTGERS a non-exclusive,
royalty-free perpetual license to install, use, modify, prepare
derivative works, incorporate into the SOFTWARE or other computer
software, distribute, and sublicense your Enhancements or derivative
works thereof, in binary and source code form.

4. FEES.  There is no license fee for the SOFTWARE.  If Licensee
wishes to receive the SOFTWARE on media, there may be a small charge
for the media and for shipping and handling.  Licensee is
responsible for any and all taxes.

5. TERMINATION.  Without prejudice to any other rights, Licensor
may terminate this Agreement if Licensee breaches any of its terms
and conditions.  Upon termination, Licensee shall destroy all
copies of the SOFTWARE.

6. PROPRIETARY RIGHTS.  Title, ownership rights, and intellectual
property rights in the Product shall remain with RUTGERS.  Licensee 
acknowledges such ownership and intellectual property rights and will 
not take any action to jeopardize, limit or interfere in any manner 
with RUTGERS' ownership of or rights with respect to the SOFTWARE.  
The SOFTWARE is protected by copyright and other intellectual 
property laws and by international treaties.  Title and related 
rights in the content accessed through the SOFTWARE is the property 
of the applicable content owner and is protected by applicable law.  
The license granted under this Agreement gives Licensee no rights to such
content.

7. DISCLAIMER OF WARRANTY.  THE SOFTWARE IS PROVIDED FREE OF 
CHARGE, AND, THEREFORE, ON AN "AS IS" BASIS, WITHOUT WARRANTY OF 
ANY KIND, INCLUDING WITHOUT LIMITATION THE WARRANTIES THAT IT 
IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE 
OR NON-INFRINGING.  THE ENTIRE RISK AS TO THE QUALITY AND 
PERFORMANCE OF THE SOFTWARE IS BORNE BY LICENSEE.  SHOULD THE 
SOFTWARE PROVE DEFECTIVE IN ANY RESPECT, THE LICENSEE AND NOT 
LICENSOR ASSUMES THE ENTIRE COST OF ANY SERVICE AND REPAIR.  
THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL PART OF 
THIS AGREEMENT.  NO USE OF THE PRODUCT IS AUTHORIZED HEREUNDER 
EXCEPT UNDER THIS DISCLAIMER.

8. LIMITATION OF LIABILITY.  TO THE MAXIMUM EXTENT PERMITTED BY
APPLICABLE LAW,  IN NO EVENT WILL LICENSOR BE LIABLE FOR ANY 
INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING 
OUT OF THE USE OF OR INABILITY TO USE THE SOFTWARE, INCLUDING, 
WITHOUT LIMITATION, DAMAGES FOR LOSS OF GOODWILL, WORK 
STOPPAGE, COMPUTER FAILURE OR MALFUNCTION, OR ANY AND ALL 
OTHER COMMERCIAL DAMAGES OR LOSSES, EVEN IF ADVISED OF THE
POSSIBILITY THEREOF. 
*/


/*!
** \file ParentChild.C
**
** \brief Implementation file for CifFile class.
*/


#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

#include "Exceptions.h"
#include "GenCont.h"
#include "CifString.h"
#include "ISTable.h"
#include "ParentChild.h"


using std::runtime_error;
using std::find;
using std::string;
using std::pair;
using std::vector;
using std::set;
using std::multiset;
using std::map;
using std::multimap;
using std::cout;
using std::endl;


ParentChild::ParentChild()
{
    _groupTableP = new ISTable("pdbx_item_linked_group");
    _groupTableP->AddColumn("category_id"); // child's category name
    _groupTableP->AddColumn("link_group_id");  // combo key group
    _groupTableP->AddColumn("label");  // atom_label_1
    _groupTableP->AddColumn("context");  // context
    _groupTableP->AddColumn("condition_id");  // context

    _groupListTableP = new ISTable("pdbx_item_linked_group_list");
    _groupListTableP->AddColumn("child_category_id"); // category name
    _groupListTableP->AddColumn("link_group_id");  // combo key group
    _groupListTableP->AddColumn("child_name");  // item name
    _groupListTableP->AddColumn("parent_name");  // item name
    _groupListTableP->AddColumn("parent_category_id");  // category name
}


ParentChild::~ParentChild()
{
    delete (_groupListTableP);
    delete (_groupTableP);
}


const vector<vector<string> >& ParentChild::GetComboKeys(
  const string& catName)
{
    return ((_parComboKeys)[catName]);
}


vector<vector<vector<string> > >& ParentChild::GetChildrenKeys(
  const vector<string>& parComboKey)
{
    return ((_relations)[parComboKey]);
}


void ParentChild::GetComboKeys(const string& parCatName,
  const unsigned int maxKeyGroup, ISTable& keysTable,
  vector<vector<string> >& comboKeys, vector<string>& parKeys)
{
    // First determine the order of keys
    // Look for all keys that have this parent and which group number is 1
    // Return result will have keys that may have single or multipe occurences

    vector<string> keyList;
    keyList.emplace_back("keyGroup");
    keyList.emplace_back("parCategory");

    vector<string> keyTarget;
    keyTarget.emplace_back("1");
    keyTarget.push_back(parCatName);

    vector<string> groupList;
    groupList.emplace_back("keyGroup");
    groupList.emplace_back("parKeyCifItem");

    vector<unsigned int> groupOne;

    keysTable.Search(groupOne, keyTarget, keyList);

    vector<string> firstComboKey;

    // Fill in the first combo key and find out the order of parent keys
    for (unsigned int itemI : groupOne)
    {
        firstComboKey.push_back(keysTable(itemI,
          "childKeyCifItem"));

        parKeys.push_back(keysTable(itemI,
          "parKeyCifItem"));
    }

    comboKeys.push_back(firstComboKey);

    for (unsigned int groupI = 2; groupI <= maxKeyGroup; ++groupI)
    {
        vector<string> nextComboKey;

        for (unsigned int parKeyI = 0; parKeyI < parKeys.size(); ++parKeyI)
        {
            vector<string> groupTarget;
            groupTarget.push_back(String::IntToString(groupI));
            groupTarget.push_back(parKeys[parKeyI]);

            unsigned int nextGroupIndex;

            nextGroupIndex = keysTable.FindFirst(groupTarget, groupList);
            if (nextGroupIndex == keysTable.GetNumRows())
            {
                // Use it from the first combo key
                nextComboKey.push_back(firstComboKey[parKeyI]);
            }
            else
            {
                nextComboKey.push_back(keysTable(nextGroupIndex,
                  "childKeyCifItem"));
            }
        }
        comboKeys.push_back(nextComboKey);
    }
}


void ParentChild::UpdateParComboKeys(const string& parName,
  vector<string>& parKeys)
{
    if (parName.empty() || parKeys.empty())
    {
        return;
    }

    // Search for the parent
    auto pos =
      _parComboKeys.find(parName);
    if (pos != _parComboKeys.end())
    {
        // Found the parent
        // Search parent for the keys in order to avoid duplication
        if (find((pos->second).begin(), (pos->second).end(), parKeys) ==
           (pos->second).end())
        {
            // These keys do not exist. Add them to the parent.
            (pos->second).push_back(parKeys);
        }
    }
    else
    {
        vector<vector<string> > tmp;
        tmp.push_back(parKeys);

        // Not found. Insert the parent with keys
        map<string, vector<vector<string> > >::value_type
          valuePair(parName, tmp);

        _parComboKeys.insert(valuePair);
    }
}


void ParentChild::UpdateRelations(vector<string>& parKeys,
  vector<vector<string> >& comboKeys)
{
    if (parKeys.empty() || comboKeys.empty())
    {
        return;
    }

    // Search for the parent
    auto pos =
      _relations.find(parKeys);
    if (pos != _relations.end())
    {
        // Found the parent keys
        // Search parent for the keys in order to avoid duplication
        if (find((pos->second).begin(), (pos->second).end(), comboKeys) ==
           (pos->second).end())
        {
            // These keys do not exist. Add them to the parent.
            (pos->second).push_back(comboKeys);
        }
    }
    else
    {
        // Not found. Insert the parent with keys
        vector<vector<vector<string> > > tmp;
        tmp.push_back(comboKeys);

        map<vector<string>,
          vector<vector<vector<string> > > >::value_type
          valuePair(parKeys, tmp);

        _relations.insert(valuePair);
    }
}


void ParentChild::AddParentCategoryToItemLinkedGroup(
  ISTable& itemLinkedGroup, ISTable& itemLinkedGroupList)
{
    vector<string> newCol;

    vector<unsigned int> rowsToDelete;

    for (unsigned int rowI = 0; rowI < itemLinkedGroup.GetNumRows(); ++rowI)
    {
        vector<string> searchCol;
        searchCol.emplace_back("child_category_id");
        searchCol.emplace_back("link_group_id");

        vector<string> searchVal;
        searchVal.push_back(itemLinkedGroup(rowI, "category_id"));
        searchVal.push_back(itemLinkedGroup(rowI, "link_group_id"));

        unsigned int found = itemLinkedGroupList.FindFirst(searchVal,
          searchCol);
        if (found == itemLinkedGroupList.GetNumRows())
        {
            cout << "Warning: Link group \"" + searchVal[1] +
              "\" of category \"" + searchVal[0] + "\" not found in table \"" +
              itemLinkedGroupList.GetName() + "\" and this entry will be "\
              "ignored." << endl;

            rowsToDelete.push_back(rowI);

            continue;
        }

        newCol.push_back(itemLinkedGroupList(found, "parent_category_id"));
    }

    itemLinkedGroup.DeleteRows(rowsToDelete);
 
    itemLinkedGroup.AddColumn("parent_category_id", newCol);
}


void ParentChild::CreateAllRelations(
  ISTable& itemLinkedGroup, ISTable& itemLinkedGroupList)
{
    for (unsigned int rowI = 0; rowI < itemLinkedGroup.GetNumRows(); ++rowI)
    {
        vector<string> searchCol;
        searchCol.emplace_back("child_category_id");
        searchCol.emplace_back("link_group_id");
        searchCol.emplace_back("parent_category_id");

        vector<string> searchVal;
        searchVal.push_back(itemLinkedGroup(rowI, "category_id"));
        searchVal.push_back(itemLinkedGroup(rowI, "link_group_id"));
        searchVal.push_back(itemLinkedGroup(rowI, "parent_category_id"));

        vector<unsigned int> found;
        itemLinkedGroupList.Search(found, searchVal, searchCol);

        vector<string> parKeys;
        for (unsigned int foundI : found)
        {
            parKeys.push_back(itemLinkedGroupList(foundI,
              "parent_name")); 
        }

#ifdef VLAD_FIX
    if (parKeys.empty())
    {
        cout << "EMPTY PARENT KEYS !!!" << endl;
        exit(1);
    }
#endif

        map<string, vector<vector<string> > > childrenKeys;

        ISTableFindPairs(childrenKeys, parKeys, itemLinkedGroupList);

        UpdateParComboKeys(itemLinkedGroup(rowI, "parent_category_id"),
          parKeys);

        for (auto & childrenKey : childrenKeys)
        {
            // pos->first is child name
            // pos->second are vector of childs combo keys
            UpdateRelations(parKeys, childrenKey.second);
        }
    }
}


void ParentChild::ISTableFindPairs(
  map<string, vector<vector<string> > >& childrenKeys,
  const vector<string>& parKeys, ISTable& itemLinkedGroupList)
{
    string parCatName;
    CifString::GetCategoryFromCifItem(parCatName, parKeys[0]);

    // Find all the combo keys in which the first parent key exists.
    vector<string> searchCol;
    searchCol.emplace_back("parent_name");

    vector<string> searchVal;
    searchVal.push_back(parKeys[0]);

    vector<unsigned int> found1;
    itemLinkedGroupList.Search(found1, searchVal, searchCol);

    for (unsigned int found1I : found1)
    {
        // See if parent key of this group matches the input parent key.
        vector<string> searchCol2;
        searchCol2.emplace_back("parent_category_id");
        searchCol2.emplace_back("link_group_id");
        searchCol2.emplace_back("child_category_id");

        vector<string> searchVal2;
        searchVal2.push_back(parCatName);
        searchVal2.push_back(itemLinkedGroupList(found1I,
          "link_group_id"));
        searchVal2.push_back(itemLinkedGroupList(found1I,
          "child_category_id"));

        vector<unsigned int> found2;
        itemLinkedGroupList.Search(found2, searchVal2, searchCol2);

        vector<string> foundParKeys;
        for (unsigned int found2I : found2)
        {
            foundParKeys.push_back(itemLinkedGroupList(found2I,
              "parent_name"));
        }

        if (!KeysMatch(parKeys, foundParKeys))
            continue;

        string currChildCat = itemLinkedGroupList(found1I,
          "child_category_id");

        // The position of child key elements is relevant to the parent keys
        vector<string> childKeys;

        // The multimap key is a parent key and the multimap value is
        // child key.
        multimap<string, string, StringLess> keysMap;

        for (unsigned int found2I : found2)
        {
            multimap<string, string, StringLess>::value_type
              valuePair(itemLinkedGroupList(found2I,
              "parent_name"), itemLinkedGroupList(found2I,
              "child_name"));
            keysMap.insert(valuePair);
        }

        // This block puts the child key to the appropriate position in
        // the vector of child keys.

        // The map key is a parent key and the map value is its order number.
        // Order number different than 0 can happen for parent keys that have
        // multiple child keys.
        map<string, unsigned int, StringLess> parKeysMap;
        for (const auto & parKey : parKeys)
        {
            unsigned int orderNum = 0;

            if (!(parKeysMap.find(parKey) == parKeysMap.end()))
            {
                orderNum = parKeysMap[parKey];
            }
 
            map<string, unsigned int, StringLess>::value_type
              valuePair(parKey, orderNum + 1);
            parKeysMap.insert(valuePair);

            pair<multimap<string, string, StringLess>::iterator,
              multimap<string, string, StringLess>::iterator> range;

            range = keysMap.equal_range(parKey);
            for (auto it =
              range.first; it != range.second; ++it)
            {
                if (static_cast<unsigned int>(distance(range.first, it)) ==
                  orderNum)
                {
                    childKeys.push_back((*it).second);
                }
            }
        } // For every parent key

        if (parKeys.size() != childKeys.size())
            cout << "PARENT AND CHILD KEYS DIFFERENT SIZE" << endl;

        UpdateMap(childrenKeys, currChildCat, childKeys);
    } // For all combo keys that contain the first parent key item
}


void ParentChild::UpdateMap(
  map<string, vector<vector<string> > >& childrenKeys,
  const string& childCat, vector<string>& childKeys)
{
    // Search for the parent
    auto pos =
      childrenKeys.find(childCat);
    if (pos != childrenKeys.end())
    {
        // Found the child
        // Search child for the keys in order to avoid duplication
        if (find((pos->second).begin(), (pos->second).end(), childKeys) ==
           (pos->second).end())
        {
            // These keys do not exist. Add them to the parent.
            (pos->second).push_back(childKeys);
        }
    }
    else
    {
        // Not found. Insert the parent with keys
        vector<vector<string> > tmp;
        tmp.push_back(childKeys);

        map<string,
          vector<vector<string> > >::value_type
          valuePair(childCat, tmp);

        childrenKeys.insert(valuePair);
    }
}


void ParentChild::GetParents(vector<vector<string> >& parParKeys,
  vector<vector<string > >& comboComboKeys, const string& childCat)
{
    // For every parent with combo keys
    for (auto & _parComboKey : _parComboKeys)
    {
        vector<vector<string> >& parKeys = _parComboKey.second;

        // For every combo parent key
        for (auto & parKey : parKeys)
        {
            // See if it exists in children

            for (auto & _relation : _relations)
            {
                if (_relation.first != parKey)
                    continue;

                vector<vector<vector<string> > >& childrenComboKeys =
                  _relation.second;

                for (auto & childComboKeys : childrenComboKeys)
                {
                    // Find the child category of these childCombo Keys
                    string childCatName;
                    CifString::GetCategoryFromCifItem(childCatName,
                      childComboKeys[0][0]);

                    if (childCatName != childCat)
                        continue;

                    for (const auto & childComboKey : childComboKeys)
                    {
                        parParKeys.push_back(parKey);
                        comboComboKeys.push_back(childComboKey);
                    }
                    // WARNIIIIIIIIIIIIIIIIIIINIG
                    // break;
                }
            }
        } 
    }
}


#ifdef VLAD_IMPLEMENT_LATER
void ParentChild::PrintAllParents(vector<vector<string> >& parParKeys,
  vector<vector<string > >& comboComboKeys, const string& childCat)
{
    // For every parent with combo keys
    for (map<string, vector<vector<string> > >::iterator pos =
      _parComboKeys.begin(); pos != _parComboKeys.end(); ++pos)
    {
        vector<vector<string> >& parKeys = pos->second;

        cout << "Parent: \"" << pos->first << "\"" << endl;

        // For every combo parent key
        for (unsigned int parKeysI = 0; parKeysI < parKeys.size(); ++parKeysI)
        {
            vector<string>& parKey = parKeys[parKeysI];
            count << "  Parent key (";
            
            for (unsigned int keyI = 0; keyI < parKey.size(); ++keyI)
            {
                cout << parKey[keyI];
                if (keyI != (parKey.size() - 1))
                    cout << ", ";
            } 

            // See if it exists in children

            for (map<vector<string>,
              vector<vector<vector<string> > > >::iterator pos2 =
              _relations.begin(); pos2 != _relations.end(); ++pos2)
            {
                if (pos2->first != parKey)
                    continue;

                vector<vector<vector<string> > >& childrenComboKeys =
                  pos2->second;

                vector<vector<string> >& childComboKeys = childrenComboKeys[0];
 
                // Find the child category of these childCombo Keys
                string childCatName;
                CifString::GetCategoryFromCifItem(childCatName,
                  childComboKeys[0][0]);

                if (childCatName != childCat)
                    continue;

                for (unsigned int kI = 0; kI < childComboKeys.size(); ++kI)
                {        
                    vector<string>& childKey = childComboKeys[kI];

                    comboComboKeys.push_back(childComboKeys[kI]);
                }
                // WARNIIIIIIIIIIIIIIIIIIINIG
                break;
            }
        } 
    }
}
#endif


void ParentChild::GetLinkGroupIdLabel(string& linkGroupIdLabel,
  const vector<string>& parKeys, const vector<string>& childKeys)
{
    linkGroupIdLabel.clear();

    string parCatName;
    CifString::GetCategoryFromCifItem(parCatName, parKeys[0]);

    string childCatName;
    CifString::GetCategoryFromCifItem(childCatName, childKeys[0]);

    vector<string> searchCol;
    searchCol.emplace_back("child_category_id");
    searchCol.emplace_back("child_name");
    searchCol.emplace_back("parent_name");
    searchCol.emplace_back("parent_category_id");

    vector<string> searchCol2;
    searchCol2.emplace_back("child_category_id");
    searchCol2.emplace_back("link_group_id");
    searchCol2.emplace_back("child_name");
    searchCol2.emplace_back("parent_name");
    searchCol2.emplace_back("parent_category_id");

    vector<string> searchVal;
    searchVal.push_back(childCatName);
    searchVal.push_back(childKeys[0]);
    searchVal.push_back(parKeys[0]);
    searchVal.push_back(parCatName);

    vector<unsigned int> found1;
    _groupListTableP->Search(found1, searchVal, searchCol);

    for (unsigned int found1I : found1)
    {
        string currLinkGroupId = (*_groupListTableP)(found1I,
          "link_group_id");

        bool matched = true;

        for (unsigned int parKeysI = 1; parKeysI < parKeys.size();
          ++parKeysI)
        {
            vector<string> searchVal2;
            searchVal2.push_back(childCatName);
            searchVal2.push_back(currLinkGroupId);
            searchVal2.push_back(childKeys[parKeysI]);
            searchVal2.push_back(parKeys[parKeysI]);
            searchVal2.push_back(parCatName);

            unsigned int found2 = _groupListTableP->FindFirst(searchVal2,
              searchCol2);
            if (found2 == _groupListTableP->GetNumRows())
            {
                matched = false;
                break;
            }
        }

        if (matched)
        {
            // Find the label for this linkGroupId
            vector<string> searchCol3;
            searchCol3.emplace_back("category_id");
            searchCol3.emplace_back("link_group_id");
            searchCol3.emplace_back("parent_category_id");

            vector<string> searchVal3;
            searchVal3.push_back(childCatName);
            searchVal3.push_back(currLinkGroupId);
            searchVal3.push_back(parCatName);

            unsigned int found3 = _groupTableP->FindFirst(searchVal3,
              searchCol3);

            if (found3 == _groupTableP->GetNumRows())
            {
                throw runtime_error("CRITICAL ERROR IN: "\
                  "ParentChild::GetLinkGroupIdLabel");
            }

            linkGroupIdLabel = (*_groupTableP)(found3, "label");
            return;
        }
    }
}


bool ParentChild::IsParKeyPresent(const vector<string>& parKey,
  const string& childCatName)
{
    for (auto & _parComboKey : _parComboKeys)
    {
        vector<vector<string> >& second = _parComboKey.second;

        for (auto & pos2 : second)
        {
            if (pos2 == parKey)
            {
                // Get its children and see if they belong to this child
                // Search for the parent
                for (auto & _relation : _relations)
                {
                    const vector<string>& pK = _relation.first;
                    if (pK != parKey)
                        continue;
    
                    vector<vector<vector<string> > >& cccK = _relation.second;
    
                    for (auto & cccI : cccK)
                    {
                        string cCat;
                        CifString::GetCategoryFromCifItem(cCat,
                          cccI[0][0]);

                        if (childCatName == cCat)
                            return (true);
                    }
                }
            }
        }
    }

    return (false);
}


bool ParentChild::IsInParentComboKeys(const std::string& itemName)
{
    string catName;
    CifString::GetCategoryFromCifItem(catName, itemName);

    const vector<vector<string> >& parComboKeys = GetComboKeys(catName);

    for (const auto & parComboKey : parComboKeys)
    {
        if (GenCont::IsInVector(itemName, parComboKey))
        {
            return (true);
        }
    }

    return (false);
}


bool ParentChild::KeysMatch(const vector<string>& firstKey,
  const vector<string>& secondKey)
{
    if (firstKey.size() != secondKey.size())
    {
        return (false);
    }

    multiset<string, StringLess> first(firstKey.begin(), firstKey.end());
    multiset<string, StringLess> second(secondKey.begin(), secondKey.end());
 
    if (first != second)
    {
        return (false);
    }

    return (true);
}
