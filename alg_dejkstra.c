#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <limits.h>


#define LOG(...) fprintf(stdout, "" __VA_ARGS__)


struct NODE
{
	char* gen_node_name;
	char* other_node_name;
	int weight;
};

void free_graph(struct NODE** weighted_graph, size_t size)
{
	if (!weighted_graph) return;
	for (size_t i = 0; i < size; ++i)
	{
		if (weighted_graph[i])
		{
			free(weighted_graph[i]->gen_node_name);
			free(weighted_graph[i]->other_node_name);
			free(weighted_graph[i]);
		}
	}
	free(weighted_graph);
	LOG("Память под граф успешно очищена.\n");
}

struct NODE** generate_weighted_graph_from_file(const char* file_name, size_t* graph_size)
{

	FILE* file_handler = fopen(file_name, "r");
	if (!file_handler)
	{
		perror("Не удалось открыть файл");
		return NULL;
	}

	fseek(file_handler, 0, SEEK_END);
	long file_size = ftell(file_handler);
	fseek(file_handler, 0, SEEK_SET);

	char* buffer = (char*)malloc(file_size + 1);
	if (!buffer)
	{
		perror("Не удалось выделить память под буфер файла");
		fclose(file_handler);
		return NULL;
	}
	size_t read_bytes = fread(buffer, 1, file_size, file_handler);
	buffer[read_bytes] = '\0';
	fclose(file_handler);

	size_t lines_count = 0;
	for (long i = 0; i < file_size; i++)
	{
		if (buffer[i] == '\n') lines_count++;
	}
	if (file_size > 0 && buffer[file_size - 1] != '\n')
	{
		lines_count++;
	}

	if (lines_count == 0) {
		fprintf(stderr, "Файл пуст или не содержит рёбер.\n");
		free(buffer);
		return NULL;
	}

	*graph_size = lines_count;
	struct NODE** weighted_graph = (struct NODE**)malloc(sizeof(struct NODE*) * lines_count);
	if (!weighted_graph)
	{
		perror("Не удалось выделить память под граф");
		free(buffer);
		return NULL;
	}

	size_t current_index = 0;
	char* line = strtok(buffer, "\n\r");

	while (line != NULL && current_index < lines_count)
	{
		char node1_name[64], node2_name[64];
		int weight;

		if (sscanf(line, " %63[^(](%d,%63[^)])", node1_name, &weight, node2_name) == 3)
		{
			weighted_graph[current_index] = (struct NODE*)malloc(sizeof(struct NODE));
			if (!weighted_graph[current_index])
			{
				perror("Не удалось выделить память под узел графа");
				free(buffer);
				free_graph(weighted_graph, current_index);
				return NULL;
			}
			weighted_graph[current_index]->gen_node_name = strdup(node1_name);
			weighted_graph[current_index]->other_node_name = strdup(node2_name);
			weighted_graph[current_index]->weight = weight;
			current_index++;
		}
		line = strtok(NULL, "\n\r");
	}

	*graph_size = current_index;
	free(buffer);
	LOG("Граф из %zu ребер успешно создан из файла.\n", *graph_size);
	return weighted_graph;
}


char** find_all_unique_nodes(struct NODE** graph, size_t edge_count, size_t* vertex_count)
{

	char** unique_nodes = (char**)malloc(sizeof(char*) * edge_count * 2);
	if (!unique_nodes)
	{
		perror("Не удалось выделить память для списка уникальных вершин");
		return NULL;
	}

	size_t count = 0;

	for (size_t i = 0; i < edge_count; i++)
	{
		char* names_to_check[2] = {graph[i]->gen_node_name, graph[i]->other_node_name};

		for (int j = 0; j < 2; j++)
		{
			bool found = false;

			for (size_t k = 0; k < count; k++)
			{
				if (strcmp(names_to_check[j], unique_nodes[k]) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				unique_nodes[count] = strdup(names_to_check[j]);
				count++;
			}
		}
	}


	char** final_nodes = (char**)realloc(unique_nodes, sizeof(char*) * count);
	if (!final_nodes && count > 0)
	{
		*vertex_count = count;
		return unique_nodes;
	}

	*vertex_count = count;
	return final_nodes;
}

void free_unique_nodes(char** nodes, size_t count)
{
	if (!nodes) return;
	for (size_t i = 0; i < count; i++)
	{
		free(nodes[i]);
	}
	free(nodes);
	LOG("Память под список вершин успешно очищена.\n");
}



void print_graph_info(struct NODE** graph, size_t edge_count, char** nodes, size_t vertex_count)
{
	if (!graph || !nodes)
	{
		LOG("Граф не загружен.\n");
		return;
	}

	LOG("\n--- Всего вершин: %zu ---\n", vertex_count);
	for (size_t i = 0; i < vertex_count; i++)
	{
		LOG("Индекс: %zu, Имя: %s\n", i, nodes[i]);
	}

	LOG("\n--- Всего рёбер: %zu ---\n", edge_count);
	for (size_t i = 0; i < edge_count; i++)
	{
		LOG("Ребро %zu: %s -> %s (вес: %d)\n", i,
		    graph[i]->gen_node_name,
      graph[i]->other_node_name,
      graph[i]->weight);
	}
	LOG("--------------------------------\n");
}




void choice_one(struct NODE*** p_weighted_graph, char*** p_unique_nodes, size_t* edge_count, size_t* vertex_count)
{
	if (*p_weighted_graph) free_graph(*p_weighted_graph, *edge_count);
	if (*p_unique_nodes) free_unique_nodes(*p_unique_nodes, *vertex_count);

	char file_name[128];
	LOG("Введите путь до файла (например, graph.txt): ");
	if (!fgets(file_name, sizeof(file_name), stdin)) return;
	file_name[strcspn(file_name, "\n")] = 0;

	*p_weighted_graph = generate_weighted_graph_from_file(file_name, edge_count);

	if (*p_weighted_graph)
	{
		LOG("Граф из %zu ребер успешно создан из файла.\n", *edge_count);
		*p_unique_nodes = find_all_unique_nodes(*p_weighted_graph, *edge_count, vertex_count);
		print_graph_info(*p_weighted_graph, *edge_count, *p_unique_nodes, *vertex_count);
	}
}




int main(int argc, char** argv)
{
	srand(time(NULL));

	struct NODE** weighted_graph = NULL;
	size_t edge_count = 0;

	char** unique_nodes = NULL;
	size_t vertex_count = 0;

	char choice[16];

	while (true)
	{
		LOG("\nПрограмма поиска кратчайшего пути по взвешенному графу.\n");
		LOG("1) Считать граф из файла\n");
		LOG("2) Сгенерировать граф\n");
		LOG("3) Запустить алгоритм Дейкстры\n");
		LOG("4) Выход\n");
		LOG("Ваш выбор: ");

		if (!fgets(choice, sizeof(choice), stdin)) break;

		if (strncmp(choice, "1", 1) == 0)
		{
			choice_one(&weighted_graph, &unique_nodes, &edge_count, &vertex_count);
		}

		else if (strncmp(choice, "4", 1) == 0)
		{
			LOG("Выход...\n");
			break;
		}
		else
		{
			LOG("Неверный ввод, попробуйте еще раз.\n");
		}
	}

	if (weighted_graph)
	{
		free_graph(weighted_graph, edge_count);
	}
	if (unique_nodes)
	{
		free_unique_nodes(unique_nodes, vertex_count);
	}


	return EXIT_SUCCESS;
}



