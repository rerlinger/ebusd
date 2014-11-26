/*
 * Copyright (C) John Baier 2014 <ebusd@johnm.de>
 *
 * This file is part of libebus.
 *
 * libebus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libebus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libebus. If not, see http://www.gnu.org/licenses/.
 */

#include "message.h"
#include <iostream>
#include <iomanip>

using namespace std;

void verify(bool expectFailMatch, string type, string input,
		bool match, string expectStr, string gotStr)
{
	if (expectFailMatch == true) {
		if (match == true)
			cout << "  failed " << type << " match >" << input
			        << "< error: unexpectedly succeeded" << endl;
		else
			cout << "  failed " << type << " match >" << input << "< OK" << endl;
	}
	else if (match == true)
		cout << "  " << type << " match >" << input << "< OK" << endl;
	else
		cout << "  " << type << " match >" << input << "< error: got >"
		        << gotStr << "<, expected >" << expectStr << "<" << endl;
}

void printErrorPos(vector<string>::iterator it, const vector<string>::iterator end, vector<string>::iterator pos)
{
	cout << "Errroneous item is here:" << endl;
	bool first = true;
	int cnt = 0;
	if (pos > it)
		pos--;
	while (it != end) {
		if (first == true)
			first = false;
		else {
			cout << ';';
			if (it <= pos) {
				cnt++;
			}
		}
		if (it < pos) {
			cnt += (*it).length();
		}
		cout << (*it++);
	}
	cout << endl;
	cout << setw(cnt) << " " << setw(0) << "^" << endl;
}

int main()
{
	// message= [type];class;name;[comment];[QQ];ZZ;PBSB;fields...
	// field=   name;[pos];type[;[divisor|values][;[unit][;[comment]]]]
	string checks[][5] = {
		// "message", "flags"
		{";;first;;;fe;0700;x;;bda", "26.10.2014", "fffe0700042610061451", "00", ""},
		{"w;;first;;;15;b5090400;date;;bda", "26.10.2014", "ff15b5090604002610061445", "00", ""},
	};
	map<string, DataField*> templates;
	Message* message = NULL;
	for (size_t i = 0; i < sizeof(checks) / sizeof(checks[0]); i++) {
		string check[5] = checks[i];
		istringstream isstr(check[0]);
		string inputStr = check[1];
		SymbolString mstr = SymbolString(check[2], false);
		SymbolString sstr = SymbolString(check[3], false);
		string flags = check[4];
		bool failedCreate = flags.find('c') != string::npos;
		bool failedPrepare = flags.find('p') != string::npos;
		bool failedPrepareMatch = flags.find('P') != string::npos;
		string item;
		vector<string> entries;

		while (getline(isstr, item, ';') != 0)
			entries.push_back(item);

		if (message != NULL) {
			delete message;
			message = NULL;
		}
		vector<string>::iterator it = entries.begin();
		result_t result = Message::create(it, entries.end(), templates, message);

		if (failedCreate == true) {
			if (result == RESULT_OK)
				cout << "\"" << check[0] << "\": failed create error: unexpectedly succeeded" << endl;
			else
				cout << "\"" << check[0] << "\": failed create OK" << endl;
			continue;
		}
		if (result != RESULT_OK) {
			cout << "\"" << check[0] << "\": create error: "
			        << getResultCode(result) << endl;
			printErrorPos(entries.begin(), entries.end(), it);
			continue;
		}
		if (message == NULL) {
			cout << "\"" << check[0] << "\": create error: NULL" << endl;
			continue;
		}
		if (it != entries.end()) {
			cout << "\"" << check[0] << "\": create error: trailing input" << endl;
			continue;
		}
		cout << "\"" << check[0] << "\": create OK" << endl;

		istringstream input(inputStr);
		SymbolString writeMstr = SymbolString();
		result = message->prepare(0xff, writeMstr, input);
		if (failedPrepare == true) {
			if (result == RESULT_OK)
				cout << "\"" << check[0] << "\": failed prepare error: unexpectedly succeeded" << endl;
			else
				cout << "\"" << check[0] << "\": failed prepare OK" << endl;
			continue;
		}

		if (result != RESULT_OK) {
			cout << "  prepare >" << inputStr << "< error: "
			        << getResultCode(result) << endl;
			continue;
		}
		cout << "  prepare >" << inputStr << "< OK" << endl;

		bool match = writeMstr==mstr;
		verify(failedPrepareMatch, "prepare", inputStr, match, mstr.getDataStr(), writeMstr.getDataStr());

		delete message;
		message = NULL;
	}

	for (map<string, DataField*>::iterator it = templates.begin(); it != templates.end(); it++)
		delete it->second;

	return 0;

}