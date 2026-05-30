#pragma once
#include "AsciGraphics/AsciGraphics.h"
#include "GenerateFunctions.h"
#include "CubeMesh.h"
#include <unordered_map>
#include <array>
#include <climits>

const int CHUNK_LENGTH = 16;
const int CHUNK_SIZE = CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH;

inline int get_chunk_position(int block_p)
{
	return (block_p / CHUNK_LENGTH) - (block_p % CHUNK_LENGTH != 0 && block_p < 0);
}

struct Block
{
	Block() {}
	Block(unsigned char Block_Type) : block_type(Block_Type) {}
	bool face[6] = {};
	unsigned char block_type = 0;
};

struct Chunk
{
	std::array<Block, CHUNK_SIZE> data{};
	int chunk_x = INT_MAX, chunk_y = INT_MAX, chunk_z = INT_MAX;

	Block& get_block_r(int x, int y, int z)
	{
		return data[x + z * CHUNK_LENGTH + y * CHUNK_LENGTH * CHUNK_LENGTH];
	}
	unsigned char get_block(int x, int y, int z)
	{
		return data[x + z * CHUNK_LENGTH + y * CHUNK_LENGTH * CHUNK_LENGTH].block_type;
	}
};

struct ChunkCoord
{
	int x, y, z;
	bool operator==(const ChunkCoord& o) const { return x == o.x && y == o.y && z == o.z; }
};

struct ChunkCoordHash
{
	size_t operator()(const ChunkCoord& c) const
	{
		return ((size_t)(c.x) * 73856093) ^ ((size_t)(c.y) * 19349663) ^ ((size_t)(c.z) * 83492791);
	}
};

class ChunkManager
{
	std::unordered_map<ChunkCoord, Chunk, ChunkCoordHash> chunks;
public:
	ChunkManager(int)
	{
		Cube::init_textures();
		Cube::init_tree();
	}
	unsigned char get_block(int x, int y, int z);
	Block& get_block_r(int x, int y, int z);
	bool does_block_exist(int block_x, int block_y, int block_z);
	bool add_block(int chunk_x, int chunk_y, int chunk_z);
	void mesh_chunk(int chunk_x, int chunk_y, int chunk_z);
	void mesh_block(int block_x, int block_y, int block_z);
	void mesh_adjacent_blocks(int block_x, int block_y, int block_z);
	void place_tree(int x, int y, int z);
	void render(Camera3D camera, float Brightness, vec3 sun_position);
};

inline bool ChunkManager::does_block_exist(int block_x, int block_y, int block_z)
{
	int chunk_x = get_chunk_position(block_x);
	int chunk_y = get_chunk_position(block_y);
	int chunk_z = get_chunk_position(block_z);
	return chunks.find({ chunk_x, chunk_y, chunk_z }) != chunks.end();
}

inline unsigned char ChunkManager::get_block(int block_x, int block_y, int block_z)
{
	int chunk_x = get_chunk_position(block_x);
	int chunk_y = get_chunk_position(block_y);
	int chunk_z = get_chunk_position(block_z);

	auto it = chunks.find({ chunk_x, chunk_y, chunk_z });
	if (it == chunks.end())
	{
		return 0;
	}
	Chunk& chunk = it->second;
	int x = block_x % CHUNK_LENGTH;
	int y = block_y % CHUNK_LENGTH;
	int z = block_z % CHUNK_LENGTH;
	x = x < 0 ? CHUNK_LENGTH + x : x;
	y = y < 0 ? CHUNK_LENGTH + y : y;
	z = z < 0 ? CHUNK_LENGTH + z : z;
	return chunk.get_block(x, y, z);
}

inline Block& ChunkManager::get_block_r(int block_x, int block_y, int block_z)
{
	int chunk_x = get_chunk_position(block_x);
	int chunk_y = get_chunk_position(block_y);
	int chunk_z = get_chunk_position(block_z);

	auto it = chunks.find({ chunk_x, chunk_y, chunk_z });
	if (it == chunks.end())
	{
		std::cout << "Incorrect Indexing in GetBlock_r function\n";
		throw;
	}
	Chunk& chunk = it->second;
	int x = block_x % CHUNK_LENGTH;
	int y = block_y % CHUNK_LENGTH;
	int z = block_z % CHUNK_LENGTH;
	x = x < 0 ? CHUNK_LENGTH + x : x;
	y = y < 0 ? CHUNK_LENGTH + y : y;
	z = z < 0 ? CHUNK_LENGTH + z : z;
	return chunk.get_block_r(x, y, z);
}

inline bool ChunkManager::add_block(int chunk_x, int chunk_y, int chunk_z)
{
	ChunkCoord key = { chunk_x, chunk_y, chunk_z };
	if (chunks.find(key) != chunks.end())
	{
		return 0;
	}

	Chunk& chunk = chunks[key];
	chunk.chunk_x = chunk_x;
	chunk.chunk_y = chunk_y;
	chunk.chunk_z = chunk_z;

	int block_x = chunk_x * CHUNK_LENGTH;
	int block_y = chunk_y * CHUNK_LENGTH;
	int block_z = chunk_z * CHUNK_LENGTH;

	for (int y = 0; y < CHUNK_LENGTH; y++)
		for (int z = 0; z < CHUNK_LENGTH; z++)
			for (int x = 0; x < CHUNK_LENGTH; x++)
				chunk.get_block_r(x, y, z).block_type = sinwave_world(block_x + x, block_y + y, block_z + z);
	return 1;
}

inline void ChunkManager::mesh_chunk(int chunk_x, int chunk_y, int chunk_z)
{
	auto it = chunks.find({ chunk_x, chunk_y, chunk_z });
	if (it == chunks.end())
	{
		std::cout << "Incorrect chunk indexing in MeshChunk\n";
		throw;
	}
	Chunk& chunk = it->second;
	int block_x = chunk_x * CHUNK_LENGTH;
	int block_y = chunk_y * CHUNK_LENGTH;
	int block_z = chunk_z * CHUNK_LENGTH;
	for (int y = 0; y < CHUNK_LENGTH; y++)
		for (int z = 0; z < CHUNK_LENGTH; z++)
			for (int x = 0; x < CHUNK_LENGTH; x++)
				mesh_block(block_x + x, block_y + y, block_z + z);
}

inline void ChunkManager::mesh_block(int block_x, int block_y, int block_z)
{
	Block& current_block = get_block_r(block_x, block_y, block_z);
	current_block.face[0] = 0;
	if (get_block(block_x, block_y, block_z - 1) == 0)
		current_block.face[0] = 1;
	current_block.face[1] = 0;
	if (get_block(block_x + 1, block_y, block_z) == 0)
		current_block.face[1] = 1;
	current_block.face[2] = 0;
	if (get_block(block_x, block_y, block_z + 1) == 0)
		current_block.face[2] = 1;
	current_block.face[3] = 0;
	if (get_block(block_x - 1, block_y, block_z) == 0)
		current_block.face[3] = 1;
	current_block.face[4] = 0;
	if (get_block(block_x, block_y - 1, block_z) == 0)
		current_block.face[4] = 1;
	current_block.face[5] = 0;
	if (get_block(block_x, block_y + 1, block_z) == 0)
		current_block.face[5] = 1;
}

inline void ChunkManager::mesh_adjacent_blocks(int block_x, int block_y, int block_z)
{
	for (int y = -1; y <= 1; y++)
		for (int z = -1; z <= 1; z++)
			for (int x = -1; x <= 1; x++)
				if (does_block_exist(block_x + x, block_y + y, block_z + z))
					mesh_block(block_x + x, block_y + y, block_z + z);
}

inline void ChunkManager::place_tree(int x, int y, int z)
{
	for (size_t i = 0; i < Cube::tree_assemble.size(); i++)
	{
		vec3 p = Cube::tree_blocks[i] + vec3(x, y, z);
		if (does_block_exist(p.x, p.y, p.z))
		{
			get_block_r(p.x, p.y, p.z) = Cube::tree_assemble[i];
			mesh_adjacent_blocks(p.x, p.y, p.z);
		}
	}
}

inline void ChunkManager::render(Camera3D camera, float Brightness, vec3 sun_position)
{
	vec3 sun_dir = sun_position.Normalize();
	float Face_Brightness[] =
	{
		Brightness * ((2 + sun_dir.dot(vec3(0, 0,-1))) / 2),
		Brightness * ((2 + sun_dir.dot(vec3(1, 0, 0))) / 2),
		Brightness * ((2 + sun_dir.dot(vec3(0, 0, 1))) / 2),
		Brightness * ((2 + sun_dir.dot(vec3(-1, 0,0))) / 2),
		Brightness * ((2 + sun_dir.dot(vec3(0,-1, 0))) / 2),
		(Brightness + 0.2f) * ((2 + sun_dir.dot(vec3(0, 1, 0))) / 2)
	};

	for (auto& [key, chunk] : chunks)
	{
		vec3 chunk_pos = vec3(chunk.chunk_x * CHUNK_LENGTH, chunk.chunk_y * CHUNK_LENGTH, chunk.chunk_z * CHUNK_LENGTH);
		chunk_pos = camera.RotXAxis() * camera.RotYAxis() * (chunk_pos - camera.position);

		if (chunk_pos.z < -CHUNK_LENGTH * 1.73) continue;

		int block_x = chunk.chunk_x * CHUNK_LENGTH;
		int block_y = chunk.chunk_y * CHUNK_LENGTH;
		int block_z = chunk.chunk_z * CHUNK_LENGTH;

		for (int y = 0; y < CHUNK_LENGTH; y++)
			for (int z = 0; z < CHUNK_LENGTH; z++)
				for (int x = 0; x < CHUNK_LENGTH; x++)
				{
					Block& block = chunk.get_block_r(x, y, z);
					if (block.block_type == 0) continue;
					for (int i = 0; i < 6; i++)
					{
						if (block.face[i] == 0) continue;
						vec3 position = vec3(x + block_x, y + block_y, z + block_z);
						vec3 vertices[] =
						{
							Cube::vertice_data[i * 4 + 0],
							Cube::vertice_data[i * 4 + 1],
							Cube::vertice_data[i * 4 + 2],
							Cube::vertice_data[i * 4 + 3]
						};
						Draw3D::plain_uv(position, vertices, Cube::block_type[6 * block.block_type + i], Face_Brightness[i], camera);
					}
				}
	}
}
