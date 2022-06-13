
#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "ModulePhysics.h"
#include "ModuleCollisions.h"
#include "ModuleParticles.h"
#include "Particle.h"
#include "EntityManager.h"
#include "Entity.h"
#include "ModulePlayer.h"
#include "Window.h"
#include "QuestManager.h"
#include "SceneMainMap.h"

#include "Defs.h"
#include "Log.h"

#include <math.h>
#include <vector>
#include <string.h>
#include <tchar.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

Map::Map(bool start_enabled) : Module(start_enabled), mapLoaded(false)
{
    name.Create("map");
}

// Destructor
Map::~Map()
{}

// L06: TODO_D 7: Ask for the value of a custom property
int Properties::GetProperty(const char* value, int defaultValue) const
{
	ListItem<Property*>* item;
	item = list.start;

	while (item != NULL)
	{
		if (item->data->name == value)
		{
			defaultValue = item->data->value;
		}
		item = item->next;
	}

	return defaultValue;
}

// Called before render is available
bool Map::Awake(pugi::xml_node& config)
{
    LOG("Loading Map Parser");
    bool ret = true;

    folder.Create(config.child("folder").child_value());

    return ret;
}

/*
bool Map::Start()
{
	mapChainsCounter = 0;

	return true;
}
*/

// Draw the map (all requried layers)
void Map::Draw()
{
	if (mapLoaded == false) return;

	// L04: DONE 5: Prepare the loop to draw all tilesets + DrawTexture()
	ListItem<MapLayer*>* mapLayerItem;
	mapLayerItem = mapData.layers.start;

	// L06: TODO 4: Make sure we draw all the layers and not just the first one
	while (mapLayerItem != NULL)
	{

		if (mapLayerItem->data->properties.GetProperty("Draw") == 1 && mapLayerItem->data->properties.GetProperty("LayerID") == 2)
		{

			for (int y = 0; y < mapLayerItem->data->height; y++)
			{
				for (int x = 0; x < mapLayerItem->data->width; x++)
				{
					// L04: DONE 9: Complete the draw function
					int gid = mapLayerItem->data->Get(x, y);

					if (gid > 0)
					{
						
						//L06: TODO 4: Obtain the tile set using GetTilesetFromTileId
						//now we always use the firt tileset in the list
						//TileSet* tileset = mapData.tilesets.start->data;
						TileSet* tileset = GetTilesetFromTileId(gid);
						
						SDL_Rect r = tileset->GetTileRect(gid);
						iPoint pos = MapToWorld(x, y);
						
						//app->render->DrawTexture(tileset->texture, pos.x, pos.y, &r);
						if (mapLayerItem->data->properties.GetProperty("Parallax") == 1)
						{
							if (mapLayerItem->data->properties.GetProperty("Reveal") == 1)
							{
								if(app->player->layerZeroReveal == false) app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + ( MAP_TILEHEIGHT - tileset->tileHeight), &r, 1);
							}
							if (mapLayerItem->data->properties.GetProperty("Reveal") == 0)
							{
								app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight), &r, 1);
							}
						}
						
						if (mapLayerItem->data->properties.GetProperty("Parallax") == 2)
						{
							app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight) /*- MapToWorldSingle(4)*/, &r, 0.5f);
						}
						
						if (mapLayerItem->data->properties.GetProperty("Parallax") == 3)
						{
							app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight) /*- MapToWorldSingle(5)*/, &r, 0.4f);
						}

						if (mapLayerItem->data->properties.GetProperty("Parallax") == 4)
						{
							app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight) /*- MapToWorldSingle(5)*/, &r, 0.3f);
						}
					}

				}
			}
		}

		mapLayerItem = mapLayerItem->next;
	}


	mapLayerItem = mapData.layers.start;
	app->player->hasBeenDrawed = false;
	for (uint i = 0; i < MAX_ENTITIES; ++i)
	{
		if (app->entity_manager->entities[i] != nullptr && app->entity_manager->entities[i]->EntityHP > 0)
		{
			app->entity_manager->entities[i]->hasBeenDrawed = false;
		}
	}
	for (uint i = 0; i < MAX_ACTIVE_PARTICLES; ++i)
	{
		Particle* particle = app->particles->particles[i];

		if (particle != nullptr && particle->isAlive)
		{
			app->particles->particles[i]->hasBeenDrawed = false;
		}
	}

	while (mapLayerItem != NULL)
	{

		if (mapLayerItem->data->properties.GetProperty("Draw") == 1 && mapLayerItem->data->properties.GetProperty("LayerID") == 1)
		{

			for (int y = 0; y < mapLayerItem->data->height; y++)
			{
				for (int x = 0; x < mapLayerItem->data->width; x++)
				{
					// L04: DONE 9: Complete the draw function
					int gid = mapLayerItem->data->Get(x, y);

					if (gid > 0)
					{

						//Draw player sorted
						TileSet* tileset = GetTilesetFromTileId(gid);

						SDL_Rect r = tileset->GetTileRect(gid);
						iPoint pos = MapToWorld(x, y);

						if (pos.y + (MAP_TILEHEIGHT - tileset->tileHeight) + 98/*98 VAR�A SEG�N LA ALTURA REAL DEL JUGADOR*/ > app->player->position.y && app->player->hasBeenDrawed == false)
						{
							SDL_Rect playerRect = app->player->currentAnimation->GetCurrentFrame();
							app->render->DrawTexture(app->player->texture, app->player->position.x - 20, app->player->position.y - 70, &playerRect);

							app->player->hasBeenDrawed = true;
						}

						//Draw entities sorted
						for (uint i = 0; i < MAX_ENTITIES; ++i)
						{
							if (app->entity_manager->entities[i] != nullptr && app->entity_manager->entities[i]->EntityHP > 0)
							{
								if (pos.y + (MAP_TILEHEIGHT - tileset->tileHeight) + 98 > app->entity_manager->entities[i]->position.y && app->entity_manager->entities[i]->hasBeenDrawed == false)
								{
									app->entity_manager->entities[i]->Draw();
									app->entity_manager->entities[i]->hasBeenDrawed = true;
								}
							}
						}

						//Draw particles sorted
						//Iterating all particle array and drawing any active particles
						for (uint i = 0; i < MAX_ACTIVE_PARTICLES; ++i)
						{
							Particle* particle = app->particles->particles[i];

							if (particle != nullptr && particle->isAlive)
							{
								if (pos.y + (MAP_TILEHEIGHT - tileset->tileHeight) + 98 > particle->position.y && app->particles->particles[i]->hasBeenDrawed == false)
								{
									app->render->DrawTexture(app->particles->texture_items, particle->position.x, particle->position.y, &(particle->anim.GetCurrentFrame()));
									app->particles->particles[i]->hasBeenDrawed = true;
								}
							}
						}

						//app->render->DrawTexture(tileset->texture, pos.x, pos.y, &r);
						if (mapLayerItem->data->properties.GetProperty("Parallax") == 1)
						{
							if (mapLayerItem->data->properties.GetProperty("Reveal") == 1)
							{
								if (app->player->layerZeroReveal == false) app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight), &r, 1);
							}
							if (mapLayerItem->data->properties.GetProperty("Reveal") == 0)
							{
								app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight), &r, 1);
							}
						}

						if (mapLayerItem->data->properties.GetProperty("Parallax") == 2)
						{
							app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight) /*- MapToWorldSingle(4)*/, &r, 0.5f);
						}

						if (mapLayerItem->data->properties.GetProperty("Parallax") == 3)
						{
							app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight) /*- MapToWorldSingle(5)*/, &r, 0.4f);
						}

						if (mapLayerItem->data->properties.GetProperty("Parallax") == 4)
						{
							app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight) /*- MapToWorldSingle(5)*/, &r, 0.3f);
						}
					}

				}
			}
		}

		mapLayerItem = mapLayerItem->next;
	}


	mapLayerItem = mapData.layers.start;

	while (mapLayerItem != NULL)
	{

		if (mapLayerItem->data->properties.GetProperty("Draw") == 1 && mapLayerItem->data->properties.GetProperty("LayerID") == 0)
		{

			for (int y = 0; y < mapLayerItem->data->height; y++)
			{
				for (int x = 0; x < mapLayerItem->data->width; x++)
				{
					// L04: DONE 9: Complete the draw function
					int gid = mapLayerItem->data->Get(x, y);

					if (gid > 0)
					{

						//L06: TODO 4: Obtain the tile set using GetTilesetFromTileId
						//now we always use the firt tileset in the list
						//TileSet* tileset = mapData.tilesets.start->data;
						TileSet* tileset = GetTilesetFromTileId(gid);

						SDL_Rect r = tileset->GetTileRect(gid);
						iPoint pos = MapToWorld(x, y);

						//app->render->DrawTexture(tileset->texture, pos.x, pos.y, &r);
						if (mapLayerItem->data->properties.GetProperty("Parallax") == 1)
						{
							if (mapLayerItem->data->properties.GetProperty("Reveal") == 1)
							{
								if (app->player->layerZeroReveal == false) app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight), &r, 1);
							}
							if (mapLayerItem->data->properties.GetProperty("Reveal") == 0)
							{
								app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight), &r, 1);
							}
						}

						if (mapLayerItem->data->properties.GetProperty("Parallax") == 2)
						{
							app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight) /*- MapToWorldSingle(4)*/, &r, 0.5f);
						}
						
						if (mapLayerItem->data->properties.GetProperty("Parallax") == 3)
						{
							app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight) /*- MapToWorldSingle(5)*/, &r, 0.4f);
						}

						if (mapLayerItem->data->properties.GetProperty("Parallax") == 4)
						{
							app->render->DrawTexture(tileset->texture, pos.x + (MAP_TILEWIDTH - tileset->tileWidth), pos.y + (MAP_TILEHEIGHT - tileset->tileHeight) /*- MapToWorldSingle(5)*/, &r, 0.3f);
						}
					}

				}
			}
		}

		mapLayerItem = mapLayerItem->next;
	}
}

// L04: DONE 8: Create a method that translates x,y coordinates from map positions to world positions
iPoint Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	if (mapData.type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x * mapData.tileWidth;
		ret.y = y * mapData.tileHeight;
	}

	// L05: TODO_D 1: Add isometric map to world coordinates
	if (mapData.type == MAPTYPE_ISOMETRIC)
	{
		ret.x = (x - y) * (mapData.tileWidth * 0.5f);
		ret.y = (x + y) * (mapData.tileHeight * 0.5f);
	}

	if (mapData.type == MAPTYPE_STAGGERED) // DESCUBRIMIENTO IMPORTANTE: ESTO NO EST� MAL. ES PROBLEMA DEL TILED QUE LO MUEVE. LO HE COMPROBADO
	{
		int offset = 0;

		if (y % 2 == 0)
		{
			offset = 0;
			ret.x = x * mapData.tileWidth * 0.9866f;
		}
		else
		{
			offset = 1;
			ret.x = x * mapData.tileWidth * 0.9866f + mapData.tileWidth * 0.5f;
		}


		if (y % 2 == 0)
		{
			if (offset == 0)ret.y = y * mapData.tileHeight * 0.97444f * 0.5f;
			if (offset == 1)ret.y = y * mapData.tileHeight * 0.97444f + mapData.tileHeight;
		}
		else
		{
			if (offset == 0)ret.y = y * mapData.tileHeight * 0.97444f + mapData.tileHeight;
			if (offset == 1)ret.y = y * mapData.tileHeight * 0.97444f * 0.5f;

		}
	}

	

	return ret;
}

int Map::MapToWorldSingle(int number) const // Only for orthogonal maptype (usar MapToWorld().x() || MapToWorld().y())
{
	int ret = number;

	if (mapData.type == MAPTYPE_ORTHOGONAL)
	{
		ret = number * mapData.tileWidth;
	}

	if (mapData.type == MAPTYPE_ISOMETRIC)
	{
		// No possible solution. Porque la X y la Y est�n deformadas.
	}

	if (mapData.type == MAPTYPE_STAGGERED)
	{
		// No possible solution. Porque la X y la Y est�n deformadas.
	}

	return ret;
}


iPoint Map::WorldToMap(int x, int y) const
{
	iPoint ret(0, 0);

	if (mapData.type == MAPTYPE_ORTHOGONAL)
	{
		x = ret.x / mapData.tileWidth;
		y = ret.y / mapData.tileHeight;
	}

	// L05: TODO_D 3: Add the case for isometric maps to WorldToMap
	if (mapData.type == MAPTYPE_ISOMETRIC)
	{
		y = (ret.y - abs(ret.x) * 0.5f) / (2 * (mapData.tileHeight * 0.5f));
		x = ret.x / ((2 * (mapData.tileHeight * 0.5f)) * (mapData.tileWidth * 0.5f)) / (mapData.tileWidth * 0.5f);
	}
	
	if (mapData.type == MAPTYPE_STAGGERED)
	{
		// Not needed yet
	}

	return ret;
}

// L06: TODO_D 3: Pick the right Tileset based on a tile id
TileSet* Map::GetTilesetFromTileId(int id) const
{
	ListItem<TileSet*>* item = mapData.tilesets.start;
	TileSet* set = item->data;

	while (item)
	{
		if (id < item->data->firstgid)
		{
			// Founded the tileset
			set = item->prev->data;
			break;
		}

		set = item->data;
		item = item->next;
	}
	
	return set;
}

// Get relative Tile rectangle
SDL_Rect TileSet::GetTileRect(int id) const
{
	SDL_Rect rect = { 0 };

	// L04: DONE 7: Get relative Tile rectangle
	int relativeId = id - firstgid;
	rect.w = tileWidth;
	rect.h = tileHeight;
	rect.x = margin + ((rect.w + spacing) * (relativeId % columns));
	rect.y = margin + ((rect.h + spacing) * (relativeId / columns));

	return rect;
}

// Called before quitting
bool Map::CleanUp()
{
    LOG("Unloading map");

    // L03: DONE 2: Make sure you clean up any memory allocated from tilesets/map
    // Remove all tilesets
	ListItem<TileSet*>* item;
	item = mapData.tilesets.start;

	while (item != NULL)
	{
		RELEASE(item->data);
		item = item->next;
	}
	mapData.tilesets.clear();

	// L04: DONE 2: clean up all layer data
	// Remove all layers
	ListItem<MapLayer*>* item2;
	item2 = mapData.layers.start;

	while (item2 != NULL)
	{
		RELEASE(item2->data);
		item2 = item2->next;
	}
	mapData.layers.clear();

    return true;
}

// Load new map
bool Map::Load(const char* filename)
{
    bool ret = true;
    SString tmp("%s%s", folder.GetString(), filename);

	pugi::xml_document mapFile; 
    pugi::xml_parse_result result = mapFile.load_file(tmp.GetString());

    if(result == NULL)
    {
        LOG("Could not load map xml file %s. pugi error: %s", filename, result.description());
        ret = false;
    }

	// Load general info
    if(ret == true)
    {
        // L03: DONE 3: Create and call a private function to load and fill all your map data
		ret = LoadMap(mapFile);
	}

    // L03: DONE 4: Create and call a private function to load a tileset
    // remember to support more any number of tilesets!
	if (ret == true)
	{
		ret = LoadTileSets(mapFile.child("map"));
	}

	// L04: DONE 4: Iterate all layers and load each of them
	// Load layer info
	if (ret == true)
	{
		ret = LoadAllLayers(mapFile.child("map"));
	}
	if (ret == true)
	{
		ret = LoadAllObjects(mapFile.child("map"));
	}
    if(ret == true)
    {
        // L03: DONE 5: LOG all the data loaded iterate all tilesets and LOG everything
		 
		LOG("Successfully parsed map XML file: %s", filename);
		LOG("width: %d", mapData.width);
		LOG("height: %d", mapData.height);
		LOG("tile width: %d", mapData.tileWidth);
		LOG("tile height: %d", mapData.tileHeight);
		if (mapData.type == MAPTYPE_ORTHOGONAL)
		{
			LOG("orientation: orthogonal");
		}
		else if (mapData.type == MAPTYPE_ISOMETRIC)
		{
			LOG("orientation: isometric");
		}

		ListItem<TileSet*>* tileset;
		tileset = mapData.tilesets.start;
		int tilesetCtr = 0;
		while (tileset != NULL)
		{
			LOG("Tileset %d", tilesetCtr +1);
			LOG("name: %s", tileset->data->name.GetString());
			LOG("first gid: %d", tileset->data->firstgid);
			LOG("margin: %d", tileset->data->margin);
			LOG("spacing: %d", tileset->data->spacing);
			LOG("tile width: %d", tileset->data->tileWidth);
			LOG("tile height: %d", tileset->data->tileHeight);
			LOG("width: %d", tileset->data->texWidth);
			LOG("height: %d", tileset->data->texHeight);

			tileset = tileset->next;
			tilesetCtr++;
		}

		// L04: DONE 4: LOG the info for each loaded layer
		ListItem<MapLayer*>* layer;
		layer = mapData.layers.start;
		int layerCtr = 0;

		while (layer != NULL)
		{
			LOG("Layer %d", layerCtr + 1);
			LOG("name: %s", layer->data->name.GetString());
			LOG("width: %d", layer->data->width);
			LOG("height: %d", layer->data->height);
			
			layerCtr++;
			layer = layer->next;
		}
    }
	/*
	if (ret == true)
	{
		ret = LoadAllObjects(mapFile.child("map"));
	}
	*/
    mapLoaded = ret;

    return ret;
}

// L03: DONE: Load map general properties
bool Map::LoadMap(pugi::xml_node mapFile)
{
	bool ret = true;
	pugi::xml_node map = mapFile.child("map");

	if (map == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		ret = false;
	}
	else
	{
		// L03: DONE 3: Load map general properties
		mapData.height = map.attribute("height").as_int();
		mapData.width = map.attribute("width").as_int();
		mapData.tileHeight = map.attribute("tileheight").as_int();
		mapData.tileWidth = map.attribute("tilewidth").as_int();

		// L05: DONE 1: Add formula to go from isometric map to world coordinates
		mapData.type = MAPTYPE_UNKNOWN;
		if (strcmp(map.attribute("orientation").as_string(), "staggered") == 0)
		{
			mapData.type = MAPTYPE_STAGGERED;
		}
		if (strcmp(map.attribute("orientation").as_string(), "isometric") == 0)
		{
			mapData.type = MAPTYPE_ISOMETRIC;
		}
		if (strcmp(map.attribute("orientation").as_string(), "orthogonal") == 0)
		{
			mapData.type = MAPTYPE_ORTHOGONAL;
		}
	}

	return ret;
}

// L03: DONE 4: Implement the LoadTileSet function to load the tileset properties
bool Map::LoadTileSets(pugi::xml_node mapFile) {

	bool ret = true;

	pugi::xml_node tileset;
	for (tileset = mapFile.child("tileset"); tileset && ret; tileset = tileset.next_sibling("tileset"))
	{
		TileSet* set = new TileSet();
		if (ret == true) ret = LoadTilesetDetails(tileset, set);
		if (ret == true) ret = LoadTilesetImage(tileset, set);
		mapData.tilesets.add(set);
	}
	
	return true; //<-- Tiene que ser true para que funcionen el resto de cosas asi que se pone fijo
}

// L03: DONE 4: Load Tileset attributes
bool Map::LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;

	// L03: DONE 4: Load Tileset attributes
	set->name.Create(tileset_node.attribute("name").as_string());
	set->firstgid = tileset_node.attribute("firstgid").as_int();
	set->tileWidth = tileset_node.attribute("tilewidth").as_int();
	set->tileHeight = tileset_node.attribute("tileheight").as_int();
	set->margin = tileset_node.attribute("margin").as_int();
	set->spacing = tileset_node.attribute("spacing").as_int();
	set->tilecount = tileset_node.attribute("tilecount").as_int();
	set->columns = tileset_node.attribute("columns").as_int();
	pugi::xml_node offset = tileset_node.child("tileoffset");

	return ret;
}

// L03: DONE 4: Load Tileset image
bool Map::LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	pugi::xml_node image = tileset_node.child("image");

	if (image == NULL)
	{
		LOG("Error parsing tileset xml file: Cannot find 'image' tag.");
		ret = false;
	}
	else
	{
		// L03: DONE 4: Load Tileset image
		SString tmp("%s%s", folder.GetString(), image.attribute("source").as_string());
		set->texture = app->tex->Load(tmp.GetString());
	}

	return ret;
}

// L04: DONE 3: Implement a function that loads a single layer layer
bool Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
	bool ret = true;

	//Load the attributes
	layer->id = node.attribute("id").as_int();
	layer->name = node.attribute("name").as_string();
	layer->width = node.attribute("width").as_int();
	layer->height = node.attribute("height").as_int();

	//L06: TODO_D 6 Call Load Properties
	LoadProperties(node, layer->properties);


	//Reserve the memory for the tile array
	layer->data = new uint[layer->width * layer->height];
	memset(layer->data, 0, layer->width * layer->height);

	//Iterate over all the tiles and assign the values
	pugi::xml_node tile;
	int i = 0;
	for (tile = node.child("data").child("tile"); tile && ret; tile = tile.next_sibling("tile"))
	{
		layer->data[i] = tile.attribute("gid").as_int();
		i++;
	}

	return ret;
}
bool Map::LoadObject(pugi::xml_node& node, MapObjects* object)
{
	bool ret = true;

	//Load the attributes
	object->id = node.attribute("id").as_int();
	object->name = node.attribute("name").as_string();
	app->map->MapToWorld(0,0).y;
	if (object->name == "collisionChains")
	{
		app->collisions->AddCollider({ app->map->MapToWorld(0,0).x, app->map->MapToWorld(0,0).y, app->map->MapToWorld(900,900).x,app->map->MapToWorld(900,900).y }, Collider::Type::NULL_COLLIDER); // No tiene sentido que est� aqu� pero tiene que estarlo

		//L06: TODO_D 6 Call Load Properties
		LoadProperties(node, object->properties);

		mapChainsCounter = 0;

		//Iterate over all the tiles and assign the values
		pugi::xml_node NewObject;

		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			object->points = NewObject.child("polygon").attribute("points").as_string();

			SString line = "";
			//SString temp = "";
			string temp = "";
			//pugi::char_t* temp = "";
			//SString pointsString = object->points;
			string pointsString = object->points;



			int sizeCounter = 0;
			bool finish = false;
			for (int i = 0; finish == false; i++)
			{

				if (object->points[i] == ',' || object->points[i] == ' ')
				{
					sizeCounter++;
				}
				if (object->points[i] == '\0')
				{
					finish = true;
				}
			}

			finish = false;

			sizeCounter++;
			int* pointsArray = new int[sizeCounter];
			//int pointsArray[999] = { 0 };
			int pointsArrayPosition = 0;

			for (int j = 0; finish == false; j++)
			{
				if (pointsString[j] != ',' && pointsString[j] != ' ')
				{
					temp += pointsString[j];
				}
				else if (pointsString[j] == ',' || pointsString[j] == ' ')
				{
					pointsArray[pointsArrayPosition] = stoi(temp);
					temp = "";
					pointsArrayPosition++;
				}
				if (pointsString[j] == '\0')
				{
					pointsArray[pointsArrayPosition] = stoi(temp);
					temp = "";
					finish = true;
				}
				//cout << temp << endl;
			}
			// if the string has gotten to it's limit create the chain

			mapChains[mapChainsCounter] = app->physics->CreateChain(NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), pointsArray, sizeCounter);


			delete[] pointsArray;

			mapChainsCounter++;
		}
	}
	
	if (app->player->entranceID == 0)
	{
		if (object->name == "startPos")
		{
			pugi::xml_node NewObject;
			for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
			{
				//app->player->position = app->map->MapToWorld(NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int());

				playerStartPos.x = NewObject.attribute("x").as_int();
				playerStartPos.y = NewObject.attribute("y").as_int();

				app->player->position.x = playerStartPos.x;
				app->player->position.y = playerStartPos.y;
			}
		}
	}

	if (app->player->entranceID == 1)
	{
		if (object->name == "entrance_1")
		{
			pugi::xml_node NewObject;
			for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
			{
				//app->player->position = app->map->MapToWorld(NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int());

				playerStartPos.x = NewObject.attribute("x").as_int();
				playerStartPos.y = NewObject.attribute("y").as_int();

				app->player->position.x = playerStartPos.x;
				app->player->position.y = playerStartPos.y;
			}
		}
	}

	if (app->player->entranceID == 2)
	{
		if (object->name == "entrance_2")
		{
			pugi::xml_node NewObject;
			for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
			{
				//app->player->position = app->map->MapToWorld(NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int());

				playerStartPos.x = NewObject.attribute("x").as_int();
				playerStartPos.y = NewObject.attribute("y").as_int();

				app->player->position.x = playerStartPos.x;
				app->player->position.y = playerStartPos.y;
			}
		}
	}

	if (app->player->entranceID == 3)
	{
		if (object->name == "entrance_3")
		{
			pugi::xml_node NewObject;
			for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
			{
				//app->player->position = app->map->MapToWorld(NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int());

				playerStartPos.x = NewObject.attribute("x").as_int();
				playerStartPos.y = NewObject.attribute("y").as_int();

				app->player->position.x = playerStartPos.x;
				app->player->position.y = playerStartPos.y;
			}
		}
	}
	
	if (object->name == "sensors_EXIT_1")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::EXIT_1);
		}
	}

	if (object->name == "sensors_EXIT_2")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::EXIT_2);
		}
	}

	if (object->name == "sensors_EXIT_3")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::EXIT_3);
		}
	}

	if (object->name == "sensors_SWITCH")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::SWITCH);
		}
	}

	if (object->name == "sensors_LAYER_ZERO")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::LAYER_ZERO);
		}
	}

	if (object->name == "sensors_INSTANT_DEATH")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::INSTANT_DEATH);
		}
	}

	if (object->name == "player")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::PLAYER, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}

	if (object->name == "switch_key")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::SWITCH_KEY, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}

	if (object->name == "switch_key2")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::SWITCH_KEY2, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}

	if (object->name == "switch_key3")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::SWITCH_KEY3, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}

	if (object->name == "switch_key4")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::SWITCH_KEY4, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}
	
	if (object->name == "switch_key5")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::SWITCH_KEY5, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}

	if (object->name == "rock_one")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::ROCK_ONE, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}

	if (object->name == "rock_two")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::ROCK_TWO, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}

	if (object->name == "fence_one")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::FENCE_ONE, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}

	if (object->name == "fence_two")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::FENCE_TWO, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}

	if (object->name == "npc_2")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::NPC2, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}

	if (app->questManager->mainQuestID == LOOK_FOR_THE_COMPUTER_2)
	{
		if (object->name == "npc")
		{
			pugi::xml_node NewObject;
			for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
			{
				app->entity_manager->AddEntity(EntityType::NPC, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
			}
		}

		if (object->name == "npc_3")
		{
			pugi::xml_node NewObject;
			for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
			{
				app->entity_manager->AddEntity(EntityType::NPC3, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
			}
		}
	}

	if (object->name == "npc_4")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::NPC4, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}

	if (object->name == "object_food")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->particles->AddParticle(app->particles->ItemFood, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2, Collider::Type::ITEM_FOOD);
		}
	}

	if (object->name == "object_health_pack")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->particles->AddParticle(app->particles->ItemHealthPack, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2, Collider::Type::ITEM_HEALTH_PACK);
		}
	}

	if (object->name == "object_radio")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->particles->AddParticle(app->particles->ItemRadio, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2, Collider::Type::ITEM_RADIO);
		}
	}

	if (object->name == "object_default_gun")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->particles->AddParticle(app->particles->ItemDefaultGun, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2, Collider::Type::ITEM_DEFAULT_GUN);
		}
	}

	if (object->name == "object_knife")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->particles->AddParticle(app->particles->ItemKnife, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2, Collider::Type::ITEM_KNIFE);
		}
	}

	if (object->name == "object_suit")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->particles->AddParticle(app->particles->ItemSuit, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2, Collider::Type::ITEM_SUIT);
		}
	}

	if (object->name == "object_short_scope_gun")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->particles->AddParticle(app->particles->ItemShortRangeGun, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2, Collider::Type::ITEM_SHORT_RANGE_GUN);
		}
	}

	if (object->name == "object_long_scope_gun")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->particles->AddParticle(app->particles->ItemLongRangeGun, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2, Collider::Type::ITEM_LONG_RANGE_GUN);
		}
	}

	if (object->name == "object_card")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->particles->AddParticle(app->particles->ItemCard, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2, Collider::Type::ITEM_CARD);
		}
	}

	if (object->name == "object_grenade")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->particles->AddParticle(app->particles->ItemGrenade, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2, Collider::Type::ITEM_GRENADE);
		}
	}

	if (object->name == "mini_boss_one")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::MINI_BOSS_ONE, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}

	if (object->name == "mini_boss_two")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->entity_manager->AddEntity(EntityType::MINI_BOSS_TWO, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
		}
	}

	if (app->questManager->mainQuestID != LOOK_FOR_THE_COMPUTER_2)
	{
		if (object->name == "zombie_standart")
		{
			pugi::xml_node NewObject;
			for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
			{
				app->entity_manager->AddEntity(EntityType::ZOMBIE_STANDART, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
			}
		}

		if (object->name == "zombie_spitter")
		{
			pugi::xml_node NewObject;
			for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
			{
				app->entity_manager->AddEntity(EntityType::ZOMBIE_SPITTER, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
			}
		}

		if (object->name == "zombie_runner")
		{
			pugi::xml_node NewObject;
			for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
			{
				app->entity_manager->AddEntity(EntityType::ZOMBIE_RUNNER, NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int() / 2, NewObject.attribute("y").as_int() - NewObject.attribute("height").as_int() / 2);
			}
		}
	}

	if (object->name == "sensors_card_reader")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::DOOR_KEY_READER);
		}
	}

	if (object->name == "sensors_doctor_note")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::DOCTOR_NOTE_MESSAGE);
		}
	}

	if (object->name == "sensors_computer")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::BASE_COMPUTER);
		}
	}

	if (object->name == "sensors_TNT_switch")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::TNT_SWITCH);
		}
	}

	if (object->name == "sensors_TNT_switch2")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::TNT_SWITCH2);
		}
	}

	if (object->name == "final_battle_Sensor")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::ENABLE_FINAL_BATTLE);
		}
	}

	if (object->name == "boss1_battle_Sensor")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::ENABLE_BOSS_1);
		}
	}

	if (object->name == "boss2_battle_Sensor")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::ENABLE_BOSS_2);
		}
	}

	if (object->name == "boss3_battle_Sensor")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			app->collisions->AddCollider({ NewObject.attribute("x").as_int(), NewObject.attribute("y").as_int(), NewObject.attribute("width").as_int(), NewObject.attribute("height").as_int() }, Collider::Type::ENABLE_BOSS_3);
		}
	}


	// Deffine level boundaries
	if (object->name == "LEVEL_AREA")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			levelAreaUpperBound = NewObject.attribute("y").as_int();
			levelAreaLowerBound = NewObject.attribute("y").as_int() + NewObject.attribute("height").as_int();
			levelAreaLeftBound = NewObject.attribute("x").as_int();
			levelAreaRightBound = NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int();
		}
	}

	// Deffine boss area boundaries
	if (object->name == "BOSS_AREA")
	{
		pugi::xml_node NewObject;
		for (NewObject = node.child("object"); NewObject && ret; NewObject = NewObject.next_sibling("object"))
		{
			bossAreaUpperBound = NewObject.attribute("y").as_int();
			bossAreaLowerBound = NewObject.attribute("y").as_int() + NewObject.attribute("height").as_int();
			bossAreaLeftBound = NewObject.attribute("x").as_int();
			bossAreaRightBound = NewObject.attribute("x").as_int() + NewObject.attribute("width").as_int();
		}
	}

	return ret;
}

// L04: DONE 4: Iterate all layers and load each of them
bool Map::LoadAllLayers(pugi::xml_node mapNode) {
	bool ret = true;
	for (pugi::xml_node layerNode = mapNode.child("layer"); layerNode && ret; layerNode = layerNode.next_sibling("layer"))
	{
		//Load the layer
		MapLayer* mapLayer = new MapLayer();
		ret = LoadLayer(layerNode, mapLayer);

		//add the layer to the map
		mapData.layers.add(mapLayer);
	}

	return ret;
}

bool Map::LoadAllObjects(pugi::xml_node mapNode) {
	bool ret = true;
	for (pugi::xml_node layerNode = mapNode.child("objectgroup"); layerNode && ret; layerNode = layerNode.next_sibling("objectgroup"))
	{
		//Load the layer
		MapObjects* objectLayer = new MapObjects();
		//if (layerNode.attribute("name").as_string == "collisions")ret = LoadObject(layerNode, objectLayer);
		ret = LoadObject(layerNode, objectLayer);
		
		//add the layer to the map
		mapData.objects.add(objectLayer);
	}

	return ret;
}

// L06: TODO_D 6: Load a group of properties from a layer and fill a list with it
bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
	bool ret = false;

	for (pugi::xml_node propertieNode = node.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
	{
		Properties::Property* p = new Properties::Property();
		p->name = propertieNode.attribute("name").as_string();
		p->value = propertieNode.attribute("value").as_int();

		properties.list.add(p);
	}
	
	return ret;
}


void Map::LoadCollidersSensors() // Old version
{
	if (mapLoaded == false) return;

	// L04: DONE 5: Prepare the loop to draw all tilesets + DrawTexture()
	ListItem<MapLayer*>* mapLayerItem;
	mapLayerItem = mapData.layers.start;

	// L06: TODO 4: Make sure we draw all the layers and not just the first one
	while (mapLayerItem != NULL)
	{

		if (mapLayerItem->data->properties.GetProperty("Draw") == 0)
		{

			for (int x = 0; x < mapLayerItem->data->width; x++)
			{
				for (int y = 0; y < mapLayerItem->data->height; y++)
				{
					// L04: DONE 9: Complete the draw function
					int gid = mapLayerItem->data->Get(x, y);

					if (gid > 0)
					{

						//L06: TODO 4: Obtain the tile set using GetTilesetFromTileId
						//now we always use the firt tileset in the list
						//TileSet* tileset = mapData.tilesets.start->data;
						TileSet* tileset = GetTilesetFromTileId(gid);

						SDL_Rect r = tileset->GetTileRect(gid);
						iPoint pos = MapToWorld(x, y);

						//app->render->DrawTexture(tileset->texture, pos.x, pos.y, &r);
						if (mapLayerItem->data->properties.GetProperty("Lava") == 1)
						{
							//app->physics->CreateColliderRectangle(pos.x + 8, pos.y + 8, 16, 16);
							collider = app->collisions->AddCollider({ pos.x, pos.y, 16, 16 }, Collider::Type::SWITCH);
						}
					}

				}
			}
		}

		mapLayerItem = mapLayerItem->next;
	}
}

void Map::DeleteCollidersSensors() // En realidad no haze falta. Esto se hace automaticamente cuando le das a disable
{
	//if (mapLoaded == false) return;

	// L04: DONE 5: Prepare the loop to draw all tilesets + DrawTexture()
	ListItem<MapLayer*>* mapLayerItem;
	mapLayerItem = mapData.layers.start;

	// L06: TODO 4: Make sure we draw all the layers and not just the first one
	while (mapLayerItem != NULL)
	{

		if (mapLayerItem->data->properties.GetProperty("Draw") == 0)
		{

			for (int x = 0; x < mapLayerItem->data->width; x++)
			{
				for (int y = 0; y < mapLayerItem->data->height; y++)
				{
					// L04: DONE 9: Complete the draw function
					int gid = mapLayerItem->data->Get(x, y);

					if (gid > 0)
					{

						//L06: TODO 4: Obtain the tile set using GetTilesetFromTileId
						//now we always use the firt tileset in the list
						//TileSet* tileset = mapData.tilesets.start->data;
						TileSet* tileset = GetTilesetFromTileId(gid);

						SDL_Rect r = tileset->GetTileRect(gid);
						iPoint pos = MapToWorld(x, y);

						//app->render->DrawTexture(tileset->texture, pos.x, pos.y, &r);
						if (mapLayerItem->data->properties.GetProperty("Lava") == 1)
						{
							/*
							if (app->collisions->RemoveCollider(app->collisions->AddCollider({ pos.x, pos.y, 16, 16 }, Collider::Type::SWITCH)) != nullptr)
							{
								app->collisions->RemoveCollider(app->collisions->AddCollider({ pos.x, pos.y, 16, 16 }, Collider::Type::SWITCH));
							}
							*/
							
							app->collisions->RemoveCollider(app->map->collider);
						}
					}

				}
			}
		}

		mapLayerItem = mapLayerItem->next;
	}
}


void Map::LoadCollidersNewer() //New Version (not needed any more)
{
	int Chains1[16] = { -0.5,0, 351.5,0, 351.5,16, 415.5,16, 415.5,32, 511.5,32, 511.5,128, -0.5,128 };
	int Chains2[16] = { -1.5,-0.5, - 1.5,-80.5, 94.5,-80.5, 94.5,-96.5, 238.5,-96.5, 238.5,-112.5, 270.5,-112.5, 270.5,-0.5 };
	int Chains3[56] = { -1.5,2.5, 350.5,2.5, 350.5,18.5, 398.5,18.5, 398.5,34.5, 478.5,34.5, 478.5,-13.5, 734.5,-13.5, 734.5,2.5, 830.5,2.5, 830.5,-13.5, 878.5,-13.5, 878.5,-29.5, 1422.5,-29.5, 1422.5,-13.5, 1678.5,-13.5, 1678.5,-29.5, 1774.5,-29.5, 1774.5,-45.5, 1934.5,-45.5, 1934.5,-29.5, 2030.5,-29.5, 2030.5,82.5, 1886.5,82.5, 1886.5,210.5, 2078,210, 2078.5,-237.5, - 1.5,-237.5 };
	int Chains4[12] = { 0.5,-0.5, 0.5,-80.5, - 207.5,-80.5, - 207.5,-96.5, -255.5,-96.5, -255.5,-0.5 };
	int Chains5[8] = { 1,1.5, 1,-94.5, - 47,-94.5, - 47,1.5 };
	int Chains6[8] = { -1,0, - 1,-112, 47,-112, 47,0 };
	int Chains7[15] = { 0,0, -2.54545,-2.90909, -44.3636,-2.90909, -46.5455,-0.545455, -46.5455,1.45455 ,-44.7273,2.90909 ,-2.18182,3.45455 - 0.181818,2 };
	int Chains8[24] = { 0,0, 1.81818,-2.72727, 13.4545,-3.09091 ,34.7273,6.54545, 45.2727,14.7273, 45.6364,18, 43.6364,19.4545, 40.5455,19.4545, 36.9091,14.7273, 14.7273,3.45455, 2.36364,3.81818, 0,1.81818 };
	int Chains9[14] = { 0,0 ,25.4545,-28.5455, 27.6364,-28.3636, 30.3636,-26.3636 ,30.1818,-24, 4.18182,3.09091, 1.27273,2.54545 };
	int Chains10[22] = { 0,0 ,2,-2.54545 ,15.4545,-2.36364, 22,1.09091, 30.7273,11.8182 ,30.5455,15.2727 ,28.7273,17.6364, 26.3636,17.6364 ,13.8182,3.63636 ,2.36364,3.63636, 0.181818,2.18182 };
	int Chains11[20] = { 0,0 ,8.90909,-8.36364, 26.5455,-17.0909 ,29.4545,-16.3636, 30.3636,-13.4545, 28.1818,-10 ,22.1818,-7.45455, 8.54545,0.545455, 3.09091,5.09091, 0.727273,3.81818 };
	int Chains12[38] = { 0,0, 33.4545,0 ,40.3636,-6.36364 ,63.2727,-16.5455, 92.3636,-16.1818 ,101.273,-7.27273 ,109.091,10.1818, 109.455,21.4545 ,106.545,22.9091 ,103.818,22.7273 ,101.273,20.1818 ,101.636,10.1818 ,91.8182,-9.27273 ,63.2727,-9.63636 ,46.9091,-1.45455, 36.7273,6.90909 ,0,6.72727 ,-1.81818,5.09091 ,-2,2.18182 };
	int Chains13[8] = { 0,0 ,16.1818,0 ,16.3636,-79.0909, 0.363636,-79.0909 };

	mapChains[0] = app->physics->CreateChain(0.5, 352, Chains1, 16);
	mapChains[1] = app->physics->CreateChain(625.5, 480.5, Chains2, 16);
	mapChains[2] = app->physics->CreateChain(1.5, 269.5, Chains3, 56);
	mapChains[3] = app->physics->CreateChain(1631.5, 480.5, Chains4, 12);
	mapChains[4] = app->physics->CreateChain(1743, 478.5, Chains5, 8);
	mapChains[5] = app->physics->CreateChain(1793, 480, Chains6, 8);
	mapChains[6] = app->physics->CreateChain(623.091, 403.455, Chains7, 15);
	mapChains[7] = app->physics->CreateChain(897.091, 371.455, Chains8, 24);
	mapChains[8] = app->physics->CreateChain(993.091, 400.909, Chains9, 14);
	mapChains[9] = app->physics->CreateChain(1040.73, 339.455, Chains10, 22);
	mapChains[10] = app->physics->CreateChain(1153.27, 402.727, Chains11, 20);
	mapChains[11] = app->physics->CreateChain(1218.18, 384.727, Chains12, 38);
	mapChains[12] = app->physics->CreateChain(-0.181818, 350.727, Chains13, 8);
}
bool Map::CreateWalkabilityMap(int& width, int& height, uchar** buffer) const
{
	bool ret = false;
	ListItem<MapLayer*>* item;
	item = mapData.layers.start;

	for (item = mapData.layers.start; item != NULL; item = item->next)
	{
		MapLayer* layer = item->data;

		if (layer->properties.GetProperty("Navigation", 0) == 1)
			continue;

		uchar* map = new uchar[layer->width * layer->height];
		memset(map, 1, layer->width * layer->height);

		for (int y = 0; y < mapData.height; ++y)
		{
			for (int x = 0; x < mapData.width; ++x)
			{
				int i = (y * layer->width) + x;

				int tileId = layer->Get(x, y);
				TileSet* tileset = (tileId > 0) ? GetTilesetFromTileId(tileId) : NULL;

				if (tileset != NULL)
				{
					map[i] = (tileId - tileset->firstgid) > 0 ? 0 : 1;
				}
			}
		}
		
		*buffer = map;
		width = mapData.width;
		height = mapData.height;
		ret = true;

		break;
	}

	return ret;
}

void Map::setTilePos(int x, int y, int width, int height)
{
	int position1;
	int position2;
	int position3;
	int position4;

	position1 = (x, y);
	position2 = (x + width, y);
	position3 = (x + width, y + height);
	position4 = (x, y + height);
}

