/*
 * =====================================================================================
 *
 *       Filename:  parser.c
 *
 *    Description:  Used to parse a turing machine initial state and automata 
 *    							description
 *
 *        Version:  1.0
 *        Created:  05/03/2020 13:50:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Thibaut MILHAUD (), thibaut.milhaud@ensiie.fr
 *   Organization:  ENSIIE - SUAI
 *
 * =====================================================================================
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define	MEM_SIZE 1<<10			/*  */

int main ( int argc, char *argv[] )
{

				FILE	*input;										/* input-file pointer */
				char	*input_file_name;					/* input-file name    */

				FILE	*output;										/* output-file pointer */
				char	*output_file_name;					/* output-file name    */

				if(argc == 3)
				{
								input_file_name = argv[1];
								output_file_name = argv[2];
				}
				else 
				{
								input_file_name = "input/base_test";
								output_file_name = "output/base_test.ka";
				}


				input	= fopen( input_file_name, "r" );
				if ( input == NULL ) {
								fprintf ( stderr, "couldn't open file '%s'; %s\n",
																input_file_name, strerror(errno) );
								exit (EXIT_FAILURE);
				}
				output	= fopen( output_file_name, "w" );
				if ( output == NULL ) {
								fprintf ( stderr, "couldn't open file '%s'; %s\n",
																output_file_name, strerror(errno) );
								exit (EXIT_FAILURE);
				}

				int link_id = 1;
				int mem_value, n ,q;
				char val, dep;

				if((fscanf ( input, "%d", &mem_value )) != 1)
				{
								fprintf ( stderr, "incorrect first line in '%s'\n",
																input_file_name);
								exit (EXIT_FAILURE);
				}
				
				if((fscanf ( input, "%d", &n )) != 1)
				{
								fprintf ( stderr, "incorrect second line in '%s'\n",
																input_file_name);
								exit (EXIT_FAILURE);
				}

				fprintf(output, 
				"%%init: %d C(val{t}, main{o}, start{o}), C(val{f}, main{o}, start{o})\n",
				MEM_SIZE);

				/*  TAPE INIT */

				fprintf(output, "%%init: 1\n");

				q = mem_value % 2;
				val = q ? 't' : 'f' ;
				mem_value /= 2;

				if(mem_value == 0)
								fprintf( output, "C(main{m}, start{s}, com[%d], val{%c}),\n", 
																link_id, val);
				else
								fprintf( output, "C(main{m}, start{o}, prev[%d], val{%c}),\n", 
																link_id, val);

				link_id ++;

				while(mem_value > 0)
				{
								q = mem_value % 2;
								val = q ? 't' : 'f';
								mem_value /= 2;

								if(mem_value == 0)
												fprintf( output,
												"C(main{m}, start{s}, com[%d], next[%d], val{%c}),\n", 
																				link_id, link_id-1, val);
								else
												fprintf( output, 
												"C(main{m}, start{o}, prev[%d], next[%d], val{%c}),\n", 
																				link_id, link_id-1, val);

								link_id ++;
				}
				
				/* HEAD */

				fprintf(output, "H(com_aut[%d], com_tape[%d]),\n", link_id, link_id-1);
				

				/* AUTOMATA */

				/* Contruction of graph
				 * an adjacency representation of the input automata 
				 		* graph + 4*(i*n + j) : transitions from i to j
						* the 2 first slots are for the FALSE transion 
						* and the two others for the TRUE one (value to write + dep)*/
				int i, j, pos;
				char graph[n*n*4];

				/* graph - INIT */
				for(i=0; i < n*n*4; i++) {graph[i] = 0;};

				/* graph - CONSTRUCT */
				for(i=0; i<2*n; i++)
				{
								if(fscanf(input, "%d %c %c",&j ,&val, &dep) != 3)
								{
												fprintf ( stderr, "incorrect %d line in '%s'\n",
																				i, input_file_name);
												exit (EXIT_FAILURE);

								}
								
								/* +2(i%2) is to select the good transition
								 * if i even -> false transiton
								 * else -> true tansition */
								pos = 4*(n*(i/2) + j) + 2*(i%2);
								
								graph[pos] = val;
								graph[pos+1] = dep;
				}

				int nb_pred[n];
				int links[8*n];
				int g_link_id = link_id + 1; /* memorize the com link from the head */

				for(i=0; i < n*8; i++) {links[i] = 0;};
				for(i=0; i < n; i++) {nb_pred[i] = 0;};

				for(i=0; i<n; i++)
				{
								int count =0;
								for(j=0; j<n; j++)
								{
												/* The FALSE transition */
												if(graph[4*(n*i+j)])
												{
																if(nb_pred[j] >= 6)
																{
																				fprintf ( stderr, 
																				"too much pred for %d in '%s'\n",
																				j, input_file_name);
																				exit (EXIT_FAILURE);
																}
																links[8*i] = g_link_id;
																links[8*j + 2 + nb_pred[j]] = g_link_id + 1;

																nb_pred[j] ++;
																g_link_id += 2;

																count ++;
												}
												/* The TRUE transition */
												if(graph[4*(n*i+j)+2])
												{
																if(nb_pred[j] >= 6)
																{
																				fprintf ( stderr, 
																				"too much pred for %d in '%s'\n",
																				j, input_file_name);
																				exit (EXIT_FAILURE);
																}

																links[8*i + 1] = g_link_id;
																links[8*j + 2 + nb_pred[j]] = g_link_id + 1;

																nb_pred[j] ++;
																g_link_id += 2;

																count ++;
												}

												if(count == 2)
																break;
								}
				}

				/* Writing the states */

				for(i=0; i<n; i++)
				{
								fprintf(output, "S(");
								for(j=2; j<8; j++)
								{
												if(links[8*i+j])
																fprintf(output, "prev%d[%d], ",
																							 	j-1, links[8*i+j]);
								}
								if(i==0)
												fprintf(output, "next0[%d], next1[%d], com[%d]),\n",
																				links[8*i], links[8*i+1], link_id);
								else 
												fprintf(output, "next0[%d], next1[%d]),\n",
																				links[8*i], links[8*i+1]);
								
								int count =0;
								for(j=0; j<n; j++)
								{
												/* The FALSE transition */
												if(graph[4*(n*i+j)])
												{
																fprintf(output,
																"T(prev[%d], next[%d], write{%c}, dep{%c})",
																links[8*i], links[8*i] + 1, 
																graph[4*(n*i+j)], graph[4*(n*i+j) + 1]);
																count ++;
																if( i != (n-1) || count < 2)
																				fprintf(output, ",\n");
												}
												/* The TRUE transition */
												if(graph[4*(n*i+j)+2])
												{
																fprintf(output,
																"T(prev[%d], next[%d], write{%c}, dep{%c})",
																links[8*i+1], links[8*i+1] + 1, 
																graph[4*(n*i+j) + 2], graph[4*(n*i+j) + 3]);
																count ++;
																if( i != (n-1) || count < 2)
																				fprintf(output, ",\n");
												}

												if(count == 2)
																break;
								}
				}

				if( fclose(input) == EOF ) {			/* close input file   */
								fprintf ( stderr, "couldn't close file '%s'; %s\n",
																input_file_name, strerror(errno) );
								exit (EXIT_FAILURE);
				}
				if( fclose(output) == EOF ) {			/* close output file   */
								fprintf ( stderr, "couldn't close file '%s'; %s\n",
																output_file_name, strerror(errno) );
								exit (EXIT_FAILURE);
				}


				return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

