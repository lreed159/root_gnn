from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import tensorflow as tf
from graph_nets import modules
from graph_nets import utils_tf
import sonnet as snt

NUM_LAYERS = 2    # Hard-code number of layers in the edge/node/global models.
LATENT_SIZE = 16  # Hard-code latent layer sizes for demos.


def make_mlp_model():
  """Instantiates a new MLP, followed by LayerNorm.

  The parameters of each new MLP are not shared with others generated by
  this function.

  Returns:
    A Sonnet module which contains the MLP and LayerNorm.
  """
  return snt.Sequential([
      snt.nets.MLP([LATENT_SIZE] * NUM_LAYERS,
                   activation=tf.nn.relu,
                   activate_final=True
                  ),
      snt.LayerNorm()
  ])

class MLPGraphIndependent(snt.AbstractModule):
  """GraphIndependent with MLP edge, node, and global models."""

  def __init__(self, name="MLPGraphIndependent"):
    super(MLPGraphIndependent, self).__init__(name=name)
    with self._enter_variable_scope():
      self._network = modules.GraphIndependent(
          edge_model_fn=make_mlp_model,
          node_model_fn=make_mlp_model,
          global_model_fn=make_mlp_model)

  def _build(self, inputs):
    return self._network(inputs)


class MLPGraphNetwork(snt.AbstractModule):
    """GraphIndependent with MLP edge, node, and global models."""
    def __init__(self, name="MLPGraphNetwork"):
        super(MLPGraphNetwork, self).__init__(name=name)
        with self._enter_variable_scope():
            self._network = modules.GraphNetwork(
                edge_model_fn=make_mlp_model,
                node_model_fn=make_mlp_model,
                global_model_fn=make_mlp_model)

    def _build(self, inputs):
        return self._network(inputs)


class GeneralClassifier(snt.AbstractModule):

  def __init__(self, name="GeneralClassifier"):
    super(GeneralClassifier, self).__init__(name=name)

    self._encoder = MLPGraphIndependent()
    self._core    = MLPGraphNetwork()
    self._decoder = MLPGraphIndependent()

    # Transforms the outputs into appropriate shapes.
    global_output_size = 1
    global_fn =lambda: snt.Sequential([
        snt.nets.MLP([LATENT_SIZE, global_output_size],
                     name='global_output'), tf.sigmoid])

    with self._enter_variable_scope():
        self._output_transform = modules.GraphIndependent(None, None, global_fn)

  def _build(self, input_op, num_processing_steps):
    latent = self._encoder(input_op)
    latent0 = latent

    output_ops = []
    for _ in range(num_processing_steps):
        core_input = utils_tf.concat([latent0, latent], axis=1)
        latent = self._core(core_input)

        decoded_op = self._decoder(latent)
        output_ops.append(self._output_transform(decoded_op))
    return output_ops
