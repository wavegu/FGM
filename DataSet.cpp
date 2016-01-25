#include "DataSet.h"
#include "Constant.h"

#include <cstdio>
#include <stdlib.h>
#include <iostream>
using namespace std;

#define MAX_BUF_SIZE 65536

void GlobalDataSet::LoadData(const char* data_file, Config* conf)
{
    char        buf[MAX_BUF_SIZE];
    DataSample* curt_sample = new DataSample();    
    char*       eof;

    vector<string>  tokens;

    FILE        *fin = fopen(data_file, "r");

	label_dict.GetId("1");
	label_dict.GetId("0");

    for (;;)
    {
        eof = fgets(buf, MAX_BUF_SIZE, fin);

        // One Sample finished. (by eof / emptyline)
        if (eof == NULL || strlen(buf) == 1)
        {
            if (curt_sample->node.size() > 0)
            {
                curt_sample->num_node = curt_sample->node.size();
                curt_sample->num_edge = curt_sample->edge.size();
				curt_sample->num_triangle = curt_sample->triangle.size();
                sample.push_back(curt_sample);
                curt_sample = new DataSample();
            }            
            if (eof == NULL) break;
            else continue;
        }

        // Parse detail information
        tokens = CommonUtil::StringTokenize(buf);

        if (tokens[0] == "#edge") //edge
        {
            DataEdge* curt_edge = new DataEdge();

            curt_edge->a = atoi(tokens[1].c_str()) - 1;
            curt_edge->b = atoi(tokens[2].c_str()) - 1;

            curt_edge->edge_type = 0;
            if (tokens.size() >= 4)
                curt_edge->edge_type = edge_type_dict.GetId( tokens[3] );

            curt_sample->edge.push_back(curt_edge);

            /* Edited by Xiaotao Gu, 2016.1 */
            if (edge_logic_weight_dict.find(curt_edge->edge_type) == edge_logic_weight_dict.end())
            {
                Logic_weight *logic_weight = new Logic_weight();
                logic_weight->weights = new double*[2];
                for (int i = 0; i < 2; i++)
                {
                    logic_weight->weights[i] = new double[2];
                    for (int j = 0; j < 2; j++)
                        logic_weight->weights[i][j] = 0.8;
                }
                edge_logic_weight_dict[curt_edge->edge_type] = logic_weight;
            }
        }
		else if (tokens[0] == "#triangle")//triangle
		{
			DataTriangle *t = new DataTriangle();

			t->a = atoi(tokens[1].c_str());
			t->b = atoi(tokens[2].c_str());
			t->c = atoi(tokens[3].c_str());
			if (tokens.size() <= 4)
				t->triangle_type = 0;
			else
				t->triangle_type = atoi(tokens[4].c_str());

			curt_sample->triangle.push_back(t);
		}
		else if (tokens[0] == "#color")
		{
			curt_sample->color[atoi(tokens[1].c_str())] = atoi(tokens[2].c_str());
		}
        /* Edited by Xiaotao Gu, 2016.1 */
        else if (tokens[0] == "#logic"){
            cout << "logic" << endl;
            int edge_type = edge_type_dict.GetIdConst(tokens[1]);
            if (edge_type < 0)
            {
                cout << "Error, no such edge: " << tokens[1] << endl;
            }
            else
            {
                Logic_weight *logic_weight = edge_logic_weight_dict[edge_type];
                int a_label = atoi(tokens[2].c_str());
                int b_label = atoi(tokens[3].c_str());
                double weight = atof(tokens[4].c_str());
                logic_weight->weights[a_label][b_label] = weight;
            }
        }
        else //node
        {
            DataNode* curt_node = new DataNode();

            char label_type = tokens[0][0];
            string label_name = tokens[0].substr(1);

            curt_node->label = label_dict.GetId(label_name);
            if (label_type == '+')
                curt_node->label_type = Enum::KNOWN_LABEL;
            else if (label_type == '?')
                curt_node->label_type = Enum::UNKNOWN_LABEL;
            else {
                fprintf(stderr, "Data format wrong! Label must start with +/?\n");
                exit ( 0 );
            }
            
            for (int i = 1; i < tokens.size(); i ++)
            {
                if (conf->has_attrib_value)
                {
                    vector<string> key_val = CommonUtil::StringSplit(tokens[i], ':');
                    curt_node->attrib.push_back( attrib_dict.GetId(key_val[0]) );
                    curt_node->value.push_back( atof(key_val[1].c_str()) );
                }
                else
                {
                    curt_node->attrib.push_back( attrib_dict.GetId(tokens[i]) );
                    curt_node->value.push_back(1.0);
                }
            }

            curt_node->num_attrib = curt_node->attrib.size();
            curt_sample->node.push_back( curt_node );
        }
    }
    
    num_label = label_dict.GetSize();
    num_attrib_type = attrib_dict.GetSize();
    num_edge_type = edge_type_dict.GetSize();
    if (num_edge_type == 0) num_edge_type = 1;

    /* Edited by Xiaotao Gu, 2016.1 */
    map<int, Logic_weight*>::iterator it;
    for (it = edge_logic_weight_dict.begin(); it != edge_logic_weight_dict.end(); ++it)
    {
        cout << "key: " << it->first << endl;
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++)
                cout << it->second->weights[i][j] << ' ';
        cout << endl;
    }

    fclose(fin);
}

void GlobalDataSet::LoadDataWithDict(const char* data_file, Config* conf, const MappingDict& ref_label_dict, const MappingDict& ref_attrib_dict, const MappingDict& ref_edge_type_dict)
{
    char        buf[MAX_BUF_SIZE];
    DataSample* curt_sample = new DataSample();    
    char*       eof;

    vector<string>  tokens;

    FILE        *fin = fopen(data_file, "r");

    
    for (;;)
    {
        eof = fgets(buf, MAX_BUF_SIZE, fin);

        // One Sample finished. (by eof / emptyline)
        if (eof == NULL || strlen(buf) == 1)
        {
            if (curt_sample->node.size() > 0)
            {
                sample.push_back(curt_sample);
                curt_sample = new DataSample();
            }            
            if (eof == NULL) break;
            else continue;
        }

        // Parse detail information
        tokens = CommonUtil::StringTokenize(buf);

        if (tokens[0] == "#edge") //edge
        {
            DataEdge* curt_edge = new DataEdge();

            curt_edge->a = atoi(tokens[1].c_str()) - 1;
            curt_edge->b = atoi(tokens[2].c_str()) - 1;

            curt_edge->edge_type = 0;
            if (tokens.size() >= 4)
                curt_edge->edge_type = ref_edge_type_dict.GetIdConst( tokens[3] );
            
            if (curt_edge->edge_type >= 0)
                curt_sample->edge.push_back( curt_edge );
        }
		else if (tokens[0] == "#triangle")
		{
			DataTriangle *t = new DataTriangle();

			t->a = atoi(tokens[1].c_str());
			t->b = atoi(tokens[2].c_str());
			t->c = atoi(tokens[3].c_str());
			if (tokens.size() <= 4)
				t->triangle_type = 0;
			else
				t->triangle_type = atoi(tokens[4].c_str());

			curt_sample->triangle.push_back(t);
		}
        else //node
        {
            DataNode* curt_node = new DataNode();
            curt_node->label = ref_label_dict.GetIdConst(tokens[0].substr(1));
            
            for (int i = 1; i < tokens.size(); i ++)
            {
                if (conf->has_attrib_value)
                {
                    vector<string> key_val = CommonUtil::StringSplit(tokens[i], ':');
                    int attrib_id = ref_attrib_dict.GetIdConst(key_val[0]);
                    if (attrib_id >= 0)
                    {
                        curt_node->attrib.push_back( attrib_id );
                        curt_node->value.push_back( atof(key_val[1].c_str()) );
                    }
                }
                else
                {
                    int attrib_id = ref_attrib_dict.GetIdConst( tokens[i] );
                    if (attrib_id >= 0)
                    {
                        curt_node->attrib.push_back( attrib_id );
                        curt_node->value.push_back(1.0);
                    }
                }
            }

            curt_sample->node.push_back( curt_node );
        }
    }
    
    num_label = ref_label_dict.GetSize();
    num_attrib_type = ref_attrib_dict.GetSize();
    num_edge_type = ref_edge_type_dict.GetSize();
    if (num_edge_type == 0) num_edge_type = 1;

    fclose(fin);
}
