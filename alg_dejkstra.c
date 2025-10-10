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


int generate_random_graph(struct NODE*** p_weighted_graph, char*** p_unique_nodes, size_t* edge_count, size_t* vertex_count)
{
	if (*p_weighted_graph) free_graph(*p_weighted_graph, *edge_count);
	if (*p_unique_nodes) free_unique_nodes(*p_unique_nodes, *vertex_count);

	char size_buf[16];
	LOG("Введите количество вершин для генерации (2-26): ");
	if (!fgets(size_buf, sizeof(size_buf), stdin)) return 1;

	char* endptr;
	errno = 0;
	long size_g = strtol(size_buf, &endptr, 10);

	if (endptr == size_buf || (*endptr != '\n' && *endptr != '\0') || errno == ERANGE)
	{
		fprintf(stderr, "Ошибка: Неверный ввод. Введите число.\n");
		return 1;
	}

	if (size_g < 2 || size_g > 26)
	{
		fprintf(stderr, "Ошибка: Количество вершин должно быть от 2 до 26.\n");
		return 10;
	}

	*vertex_count = (size_t)size_g;


	*p_unique_nodes = (char**)malloc(sizeof(char*) * (*vertex_count));
	if (!*p_unique_nodes) { perror("malloc failed"); return 1; }
	for (size_t i = 0; i < *vertex_count; ++i)
	{
		char name[2] = {(char)('A' + i), '\0'};
		(*p_unique_nodes)[i] = strdup(name);
		if (!(*p_unique_nodes)[i]) { perror("strdup failed"); return 1; }
	}


	size_t connecting_edges = *vertex_count - 1;
	size_t extra_edges = *vertex_count / 2;
	*edge_count = connecting_edges + extra_edges;

	*p_weighted_graph = (struct NODE**)malloc(sizeof(struct NODE*) * (*edge_count));
	if (!*p_weighted_graph) { perror("malloc failed"); return 1; }

	size_t current_edge_idx = 0;


	for (size_t i = 0; i < connecting_edges; ++i)
	{
		(*p_weighted_graph)[current_edge_idx] = (struct NODE*)malloc(sizeof(struct NODE));
		(*p_weighted_graph)[current_edge_idx]->gen_node_name = strdup((*p_unique_nodes)[i]);
		(*p_weighted_graph)[current_edge_idx]->other_node_name = strdup((*p_unique_nodes)[i + 1]);
		(*p_weighted_graph)[current_edge_idx]->weight = (rand() % 50) + 1; // Вес от 1 до 50
		current_edge_idx++;
	}


	for (size_t i = 0; i < extra_edges; ++i)
	{
		size_t v1_idx, v2_idx;
		do
		{
			v1_idx = rand() % *vertex_count;
			v2_idx = rand() % *vertex_count;
		} while (v1_idx == v2_idx);

		(*p_weighted_graph)[current_edge_idx] = (struct NODE*)malloc(sizeof(struct NODE));
		(*p_weighted_graph)[current_edge_idx]->gen_node_name = strdup((*p_unique_nodes)[v1_idx]);
		(*p_weighted_graph)[current_edge_idx]->other_node_name = strdup((*p_unique_nodes)[v2_idx]);
		(*p_weighted_graph)[current_edge_idx]->weight = (rand() % 50) + 1;
		current_edge_idx++;
	}

	LOG("\nСгенерирован граф с %zu вершинами и %zu ребрами.\n", *vertex_count, *edge_count);
	print_graph_info(*p_weighted_graph, *edge_count, *p_unique_nodes, *vertex_count);
	return 0;
}



int get_node_index(const char* name, char** unique_nodes, size_t vertex_count)
{
	for (size_t i = 0; i < vertex_count; i++)
	{
		if (strcmp(name, unique_nodes[i]) == 0)
		{
			return i;
		}
	}
	return -1;
}


int find_min_distance_node(int distances[], bool visited[], size_t vertex_count)
{
	int min_dist = INT_MAX;
	int min_index = -1;

	for (size_t v = 0; v < vertex_count; v++)
	{
		if (!visited[v] && distances[v] <= min_dist)
		{
			min_dist = distances[v];
			min_index = v;
		}
	}
	return min_index;
}


void print_path(int parent[], int j, char** unique_nodes)
{
	if (parent[j] == -1)
	{
		LOG("%s", unique_nodes[j]);
		return;
	}
	print_path(parent, parent[j], unique_nodes);
	LOG(" -> %s", unique_nodes[j]);
}



void algorithm_dijkstra(struct NODE** graph, size_t edge_count, char** unique_nodes, size_t vertex_count)
{
	if (!graph)
	{
		LOG("Ошибка: Граф не загружен. Сначала сгенерируйте или загрузите граф.\n");
		return;
	}

	// 1. Запрашиваем у пользователя начальную и конечную вершины
	char start_node_name[64], end_node_name[64];
	LOG("Список доступных вершин:\n");
	for (size_t i = 0; i < vertex_count; i++)
	{
		LOG("%s ", unique_nodes[i]);
	}
	LOG("\nВведите имя начальной вершины: ");
	scanf("%63s", start_node_name);
	LOG("Введите имя конечной вершины: ");
	scanf("%63s", end_node_name);

	while (getchar() != '\n');


	int start_idx = get_node_index(start_node_name, unique_nodes, vertex_count);
	int end_idx = get_node_index(end_node_name, unique_nodes, vertex_count);

	if (start_idx == -1 || end_idx == -1)
	{
		LOG("Ошибка: Одна или обе вершины не найдены в графе.\n");
		return;
	}

	// 2. Создаём матрицу смежности из списка рёбер
	int** adj_matrix = (int**)malloc(vertex_count * sizeof(int*));
	for (size_t i = 0; i < vertex_count; i++)
	{
		adj_matrix[i] = (int*)calloc(vertex_count, sizeof(int)); // calloc инициализирует нулями
	}

	for (size_t i = 0; i < edge_count; i++)
	{
		int u = get_node_index(graph[i]->gen_node_name, unique_nodes, vertex_count);
		int v = get_node_index(graph[i]->other_node_name, unique_nodes, vertex_count);
		if (u != -1 && v != -1) {
			adj_matrix[u][v] = graph[i]->weight;

		}
	}

	int* distances = (int*)malloc(vertex_count * sizeof(int));
	bool* visited = (bool*)malloc(vertex_count * sizeof(bool));
	int* parent = (int*)malloc(vertex_count * sizeof(int));

	for (size_t i = 0; i < vertex_count; i++)
	{
		distances[i] = INT_MAX;
		visited[i] = false;
		parent[i] = -1;
	}
	distances[start_idx] = 0;

	// 4. Основной цикл алгоритма
	for (size_t count = 0; count < vertex_count - 1; count++)
	{
		int u = find_min_distance_node(distances, visited, vertex_count);
		if (u == -1)
		{ // Если оставшиеся вершины недостижимы
			break;
		}

		visited[u] = true;

		// Обновляем расстояния до смежных вершин
		for (size_t v = 0; v < vertex_count; v++)
		{
			if (!visited[v] && adj_matrix[u][v] > 0 && distances[u] != INT_MAX &&
				distances[u] + adj_matrix[u][v] < distances[v])
			{
				distances[v] = distances[u] + adj_matrix[u][v];
				parent[v] = u;
			}
		}
	}

	// 5. Вывод результата
	LOG("\n--- Результат работы алгоритма Дейкстры ---\n");
	if (distances[end_idx] == INT_MAX)
	{
		LOG("Путь от вершины %s до %s не существует.\n", start_node_name, end_node_name);
	}
	else
	{
		LOG("Кратчайшее расстояние от %s до %s: %d\n", start_node_name, end_node_name, distances[end_idx]);
		LOG("Путь: ");
		print_path(parent, end_idx, unique_nodes);
		LOG("\n");
	}
	LOG("------------------------------------------\n");

	// 6. Очистка памяти
	for (size_t i = 0; i < vertex_count; i++)
	{
		free(adj_matrix[i]);
	}
	free(adj_matrix);
	free(distances);
	free(visited);
	free(parent);
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
		else if (strncmp(choice, "2", 1) == 0)
		{
			int res = generate_random_graph(&weighted_graph, &unique_nodes, &edge_count, &vertex_count);
			if (res != 0)
			{
				LOG("Во время генерации произошла ошибка.\n");
			}
		}
		else if (strncmp(choice, "3", 1) == 0) // Новый обработчик
		{
			algorithm_dijkstra(weighted_graph, edge_count, unique_nodes, vertex_count);
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



