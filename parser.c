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
				int m, n ,q;
				char val, dep;
				
				if((fscanf ( input, "%d %d", &n, &m )) != 2)
				{
								fprintf ( stderr, "incorrect first line in '%s'\n",
																input_file_name);
								exit (EXIT_FAILURE);
				}
				
				int i, j, pos;
				char graph[n*n*6];
				
				int nb_pred[n];
				int links[9*n];

				fprintf(output, 
				"%%init: %d C(val{s}, main{o}, start{o})\n",
				MEM_SIZE);


				/*  TAPE INIT */

				char mem[m+1];
				if((fscanf ( input, "%s", mem)) != 1)
				{
								fprintf ( stderr, "incorrect second line in '%s'\n",
																input_file_name);
								exit (EXIT_FAILURE);
				}

				printf("%s", mem);

				fprintf(output, "%%init: 1\n");	

				if(m==1)
				{
								fprintf(output, "C(main{m}, start{s}, com{%d}, val{%c}),\n",
																m, mem[0]);
								link_id = 1;
				}
				else
				{
								fprintf(output, "C(main{m}, start{s}, prev[%d], val{%c}),\n",
																1, mem[m-1]);

								for(i = 2; i < m; i++)
								{
												fprintf(output,
												"C(main{m}, start{s}, prev[%d], next[%d], val{%c}),\n",
																				i, i-1, mem[m-i]);
								}

								fprintf(output, 
								"C(main{m}, start{s}, com{%d}, next{%d}, val{%c}),\n",
																m, m-1, mem[0]);

								link_id = m;
				}
				/* HEAD */

				fprintf(output, "H(com_aut[%d], com_tape[%d]),\n", link_id, link_id-1);
				

				/* AUTOMATA */

				/* Contruction of graph
				 * an adjacency representation of the input automata 
				 		* graph + 6*(i*n + j) : transitions from i to j
						* the 2 first slots are for the FALSE transion 
						* and the two others for the TRUE one (value to write + dep)*/

				/* graph - INIT */
				for(i=0; i < n*n*6; i++) {graph[i] = 0;};

				/* graph - CONSTRUCT */
				for(i=0; i<3*n; i++)
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
								pos = 6*(n*(i/3) + j) + 2*(i%3);
								
								graph[pos] = val;
								graph[pos+1] = dep;
				}

				int g_link_id = link_id + 1; /* memorize the com link from the head */

				for(i=0; i < n*9; i++) {links[i] = 0;};
				for(i=0; i < n; i++) {nb_pred[i] = 0;};

				for(i=0; i<n; i++)
				{
								int count =0;
								for(j=0; j<n; j++)
								{
												/* The FALSE transition */
												if(graph[6*(n*i+j)])
												{
																if(nb_pred[j] >= 6)
																{
																				fprintf ( stderr, 
																				"too much pred for %d in '%s'\n",
																				j, input_file_name);
																				exit (EXIT_FAILURE);
																}
																links[9*i] = g_link_id;
																links[9*j + 3 + nb_pred[j]] = g_link_id + 1;

																nb_pred[j] ++;
																g_link_id += 2;

																count ++;
												}
												/* The TRUE transition */
												if(graph[6*(n*i+j)+2])
												{
																if(nb_pred[j] >= 6)
																{
																				fprintf ( stderr, 
																				"too much pred for %d in '%s'\n",
																				j, input_file_name);
																				exit (EXIT_FAILURE);
																}

																links[9*i + 1] = g_link_id;
																links[9*j + 3 + nb_pred[j]] = g_link_id + 1;

																nb_pred[j] ++;
																g_link_id += 2;

																count ++;
												}
												/* The SEP transition */
												if(graph[6*(n*i+j)+4])
												{
																if(nb_pred[j] >= 6)
																{
																				fprintf ( stderr, 
																				"too much pred for %d in '%s'\n",
																				j, input_file_name);
																				exit (EXIT_FAILURE);
																}

																links[9*i + 2] = g_link_id;
																links[9*j + 3 + nb_pred[j]] = g_link_id + 1;

																nb_pred[j] ++;
																g_link_id += 2;

																count ++;
												}

												if(count == 3)
																break;
								}
				}

				/* Writing the states */

				for(i=0; i<n; i++)
				{
								int id = 9*i;

								fprintf(output, "S(");
								for(j=3; j<9; j++)
								{
												if(links[id+j])
																fprintf(output, "prev%d[%d], ",
																							 	j-1, links[id+j]);
								}


								if(i==0)
												fprintf(output, "next0[%d], next1[%d], next2[%d] com[%d]),\n",
																				links[id], links[id+1], links[id+2], link_id);
								else 
												fprintf(output, "next0[%d], next1[%d], next2[%d]),\n",
																				links[id], links[id+1], links[id+2]);
								
								int count =0;
								for(j=0; j<n; j++)
								{
												int idg = 6*(i*n+j);
												/* The FALSE transition */
												if(graph[idg])
												{
																fprintf(output,
																"T(prev[%d], next[%d], write{%c}, dep{%c})",
																links[id], links[id] + 1, 
																graph[idg], graph[idg + 1]);
																count ++;
																if( i != (n-1) || count < 3)
																				fprintf(output, ",\n");
												}
												/* The TRUE transition */
												if(graph[idg+2])
												{
																fprintf(output,
																"T(prev[%d], next[%d], write{%c}, dep{%c})",
																links[id+1], links[id+1] + 1, 
																graph[idg + 2], graph[idg + 3]);
																count ++;
																if( i != (n-1) || count < 3)
																				fprintf(output, ",\n");
												}
												/* The SEP transition */
												if(graph[idg+4])
												{
																fprintf(output,
																"T(prev[%d], next[%d], write{%c}, dep{%c})",
																links[id+2], links[id+2] + 1, 
																graph[idg + 4], graph[idg + 5]);
																count ++;
																if( i != (n-1) || count < 3)
																				fprintf(output, ",\n");
												}

												if(count == 3)
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

