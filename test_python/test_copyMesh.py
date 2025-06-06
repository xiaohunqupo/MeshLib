import pytest
from helper import *


def test_copy_mesh():
    torus = mrmesh.makeOuterHalfTorus(2, 1, 10, 10, None)

    copyMesh = mrmesh.copyMesh(torus)

    assert copyMesh == torus


def test_copy_part_mesh():
    torus = mrmesh.makeOuterHalfTorus(2, 1, 10, 10, None)
    copyMesh = mrmesh.Mesh()
    copyMesh.addMesh(torus)
    assert copyMesh.topology.numValidFaces() == torus.topology.numValidFaces()

    copyMesh.addMesh(torus)

    assert copyMesh.topology.numValidFaces() == 2 * torus.topology.numValidFaces()
