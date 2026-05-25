import re
def to_snake_case(name):
    if '.' in name:
        base, ext = name.rsplit('.', 1)
        s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', base)
        s2 = re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()
        return f"{re.sub(r'[\s\-]+', '_', s2).strip('_')}.{ext.lower()}"
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    s2 = re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()
    return re.sub(r'[\s\-]+', '_', s2).strip('_')
import yaml
import os
from generation.engine_generation import EngineGeneration


class GeneratorGraphViz(EngineGeneration):

    def __init__(self, racine_project):
        super().__init__(racine_project)
        self._fichier_graphviz_yaml = os.path.join(
            racine_project, "include", "compiler", "ast", "yaml_ast", "graphviz.yaml"
        )
        self._dossier_entete = self._chemin_generation_include(
            "compiler", "visitor", "ast_graph_viz"
        )
        self._dossier_source = self._chemin_generation_src(
            "compiler", "visitor", "ast_graph_viz"
        )

    def generate(self):
        nodes = self._loadr_nodes_yaml()
        noms = list(nodes.keys())
        self._rendre_et_ecrire("visitor_graphviz.h.j2", os.path.join(self._dossier_entete, "general_visitor_graph_viz.h"), nodes=noms)

        methodes = self._preparer_methodes(nodes)
        self._rendre_et_ecrire("visitor_graphviz.cpp.j2", os.path.join(self._dossier_source, "general_visitor_graph_viz.cpp"), methodes=methodes)

    def _preparer_methodes(self, nodes):
        methodes = []
        config = self._loadr_config_graphviz()
        for nom, definition in nodes.items():
            definition = definition or {}
            champs = definition.get("champs", {})
            label = self._deduire_label(nom, champs, config["labels_speciaux"])
            traversables = self._extraire_traversables(champs)
            for e in config["childs_herites"].get(nom, []):
                traversables.append((e["getter"], e["type"]))
            methodes.append((nom, label, traversables))
        return methodes

    def _loadr_config_graphviz(self):
        if not os.path.exists(self._fichier_graphviz_yaml):
            return {"labels_speciaux": {}, "childs_herites": {}}
        with open(self._fichier_graphviz_yaml, "r", encoding="utf-8") as f:
            donnees = yaml.safe_load(f) or {}
        return {
            "labels_speciaux": donnees.get("labels_speciaux", {}),
            "childs_herites": donnees.get("childs_herites", {})
        }

    @staticmethod
    def _deduire_label(nom_node, champs, labels_speciaux):
        if nom_node in labels_speciaux:
            return labels_speciaux[nom_node]

        for nom_champ, type_champ in champs.items():
            getter = "get" + nom_champ[0].upper() + nom_champ[1:]
            if type_champ == "std::string":
                return f'"{nom_node}: " + node{nom_node}Data.{getter}()'
            if type_champ == "Token":
                return f'node{nom_node}Data.{getter}().value.str()'

        return f'"{nom_node}"'
