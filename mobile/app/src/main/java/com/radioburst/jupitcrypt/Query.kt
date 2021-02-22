/*
 * This file is part of jupitcrypt. (https://github.com/radioburst/jupitcrypt)
 * Copyright (c) 2021 Andreas Dorrer.
 *
 * jupitcrypt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * jupitcrypt is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with jupitcrypt. If not, see <http://www.gnu.org/licenses/>.
 */

package com.radioburst.jupitcrypt

class Query(
    val filters: List<String>?
) {

    private constructor(builder: Builder) : this(builder.filters)

    companion object {
        inline fun build(block: Builder.() -> Unit) = Builder().apply(block).build()
    }

    override fun toString(): String {
        return filters?.joinToString(" and ") ?: super.toString()
    }

    class Builder {
        var filters: List<String>? = null

        fun build() = Query(this)
    }
}
