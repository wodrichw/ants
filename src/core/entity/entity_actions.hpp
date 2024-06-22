#pragma once

class Map;
struct MapEntity;
class Inventory;

void handle_dig(Map&, MapEntity&, Inventory&, long dx, long dy);
